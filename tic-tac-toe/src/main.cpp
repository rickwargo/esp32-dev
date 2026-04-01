#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "secrets.h"

WebServer server(80);

// ── Game state ────────────────────────────────────────────────────────────────

static const uint8_t WIN_LINES[8][3] = {
    {0,1,2}, {3,4,5}, {6,7,8},  // rows
    {0,3,6}, {1,4,7}, {2,5,8},  // cols
    {0,4,8}, {2,4,6}            // diagonals
};

struct Game {
    char board[9];   // 'X', 'O', or 0 (empty)
    char current;    // 'X' or 'O'
    char winner;     // 'X', 'O', 'D' (draw), or 0 (ongoing)
    int  winLine[3]; // winning cell indices, or {-1,-1,-1}
    bool vsAI;
} g;

void gameReset(bool ai = false) {
    memset(g.board, 0, sizeof(g.board));
    g.current        = 'X';
    g.winner         = 0;
    g.winLine[0]     = g.winLine[1] = g.winLine[2] = -1;
    g.vsAI           = ai;
}

// Returns 'X'/'O' (winner), 'D' (draw), or 0 (ongoing).
// Also sets g.winLine when a winner is found.
char checkWinner() {
    for (const auto& l : WIN_LINES) {
        if (g.board[l[0]] &&
            g.board[l[0]] == g.board[l[1]] &&
            g.board[l[1]] == g.board[l[2]]) {
            g.winLine[0] = l[0];
            g.winLine[1] = l[1];
            g.winLine[2] = l[2];
            return g.board[l[0]];
        }
    }
    g.winLine[0] = g.winLine[1] = g.winLine[2] = -1;
    for (int i = 0; i < 9; i++) if (!g.board[i]) return 0;
    return 'D';
}

// ── Minimax AI (O maximises, X minimises) ────────────────────────────────────

int minimax(char b[9], bool isAI, int depth) {
    for (const auto& l : WIN_LINES) {
        if (b[l[0]] && b[l[0]] == b[l[1]] && b[l[1]] == b[l[2]])
            return (b[l[0]] == 'O') ? (10 - depth) : (depth - 10);
    }
    bool full = true;
    for (int i = 0; i < 9; i++) if (!b[i]) { full = false; break; }
    if (full) return 0;

    int best = isAI ? -100 : 100;
    char mark = isAI ? 'O' : 'X';
    for (int i = 0; i < 9; i++) {
        if (!b[i]) {
            b[i] = mark;
            int score = minimax(b, !isAI, depth + 1);
            b[i] = 0;
            best = isAI ? max(best, score) : min(best, score);
        }
    }
    return best;
}

int bestAIMove() {
    int best = -100, move = -1;
    for (int i = 0; i < 9; i++) {
        if (!g.board[i]) {
            g.board[i] = 'O';
            int score = minimax(g.board, false, 0);
            g.board[i] = 0;
            if (score > best) { best = score; move = i; }
        }
    }
    return move;
}

// ── JSON serialiser ───────────────────────────────────────────────────────────

String stateJson() {
    String j;
    j.reserve(120);
    j = F("{\"board\":[");
    for (int i = 0; i < 9; i++) {
        j += '"';
        if (g.board[i]) j += (char)g.board[i];
        j += '"';
        if (i < 8) j += ',';
    }
    j += F("],\"current\":\""); j += (char)g.current;
    j += F("\",\"winner\":\"");
    if (g.winner) j += (char)g.winner;
    j += F("\",\"winLine\":[");
    j += g.winLine[0]; j += ',';
    j += g.winLine[1]; j += ',';
    j += g.winLine[2];
    j += F("],\"mode\":\"");
    j += g.vsAI ? F("AI") : F("PvP");
    j += F("\"}");
    return j;
}

// ── Embedded HTML ─────────────────────────────────────────────────────────────

static const char HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Tic Tac Toe</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', sans-serif;
      background: #1a1a2e;
      color: #eee;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 1.5rem;
      padding: 1rem;
    }
    h1 { font-size: 2rem; letter-spacing: 3px; color: #e94560; }
    #status { font-size: 1.1rem; min-height: 1.6em; text-align: center; }
    #board {
      display: grid;
      grid-template-columns: repeat(3, 100px);
      grid-template-rows: repeat(3, 100px);
      gap: 8px;
    }
    .cell {
      background: #16213e;
      border: 2px solid #0f3460;
      border-radius: 10px;
      font-size: 3rem;
      font-weight: 700;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      transition: background .15s, transform .1s;
      user-select: none;
    }
    .cell.X { color: #e94560; cursor: default; }
    .cell.O { color: #4ecca3; cursor: default; }
    .cell:not(.X):not(.O):hover { background: #0f3460; transform: scale(1.04); }
    .cell.win { background: #0f3460; box-shadow: 0 0 20px #e9456066; }
    .controls { display: flex; gap: .8rem; flex-wrap: wrap; justify-content: center; }
    button {
      padding: .6rem 1.4rem;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      font-size: .9rem;
      font-weight: 600;
      transition: opacity .15s;
    }
    button:hover { opacity: .8; }
    #newBtn  { background: #e94560; color: #fff; }
    #modeBtn { background: #0f3460; color: #eee; border: 1px solid #e94560; }
    footer { font-size: .78rem; color: #555; }
  </style>
</head>
<body>
  <h1>TIC TAC TOE</h1>
  <div id="status"></div>
  <div id="board"></div>
  <div class="controls">
    <button id="newBtn"  onclick="newGame()">New Game</button>
    <button id="modeBtn" onclick="toggleMode()"></button>
  </div>
  <footer id="footer"></footer>

  <script>
    let state = {
      board: Array(9).fill(''),
      current: 'X',
      winner: '',
      winLine: [-1, -1, -1],
      mode: 'PvP'
    };

    function render() {
      const boardEl = document.getElementById('board');
      boardEl.innerHTML = '';
      state.board.forEach((v, i) => {
        const cell = document.createElement('div');
        let cls = 'cell';
        if (v) cls += ' ' + v;
        if (state.winLine.includes(i)) cls += ' win';
        cell.className = cls;
        cell.textContent = v;
        if (!state.winner && !v) cell.onclick = () => move(i);
        boardEl.appendChild(cell);
      });

      const statusEl = document.getElementById('status');
      if (state.winner === 'D')      statusEl.textContent = "It's a draw!";
      else if (state.winner)         statusEl.textContent = 'Player ' + state.winner + ' wins! \uD83C\uDF89';
      else                           statusEl.textContent = "Player " + state.current + "'s turn";

      document.getElementById('modeBtn').textContent =
        state.mode === 'AI' ? 'vs Human' : 'vs AI';
    }

    async function applyState(res) { state = await res.json(); render(); }

    async function move(i)     { await applyState(await fetch('/move?cell=' + i, { method: 'POST' })); }
    async function newGame()   { await applyState(await fetch('/reset?mode=' + state.mode, { method: 'POST' })); }
    async function toggleMode(){ await applyState(await fetch('/reset?mode=' + (state.mode === 'AI' ? 'PvP' : 'AI'), { method: 'POST' })); }

    // Poll so two devices can share the same game
    setInterval(async () => {
      const res = await fetch('/state');
      state = await res.json();
      render();
    }, 2000);

    // Boot
    fetch('/state').then(applyState).then(() =>
      fetch('/ip').then(r => r.text()).then(ip =>
        document.getElementById('footer').textContent = 'ESP32-S3 Super Mini \u2022 ' + ip
      )
    );
  </script>
</body>
</html>
)HTML";

// ── Route handlers ────────────────────────────────────────────────────────────

void handleRoot() {
    server.sendHeader(F("Cache-Control"), F("no-cache"));
    server.send_P(200, "text/html", HTML);
}

void handleState() {
    server.send(200, F("application/json"), stateJson());
}

void handleMove() {
    if (g.winner) { server.send(200, F("application/json"), stateJson()); return; }

    if (!server.hasArg("cell")) {
        server.send(400, F("text/plain"), F("missing cell"));
        return;
    }
    int cell = server.arg("cell").toInt();
    if (cell < 0 || cell > 8 || g.board[cell]) {
        server.send(200, F("application/json"), stateJson());
        return;
    }

    // Place the human move
    g.board[cell] = g.current;
    g.winner = checkWinner();

    if (!g.winner) {
        g.current = (g.current == 'X') ? 'O' : 'X';

        // AI responds immediately when it's O's turn
        if (g.vsAI && g.current == 'O') {
            int m = bestAIMove();
            if (m >= 0) {
                g.board[m] = 'O';
                g.winner = checkWinner();
                if (!g.winner) g.current = 'X';
            }
        }
    }

    server.send(200, F("application/json"), stateJson());
}

void handleReset() {
    bool ai = server.hasArg("mode") && server.arg("mode") == "AI";
    gameReset(ai);
    server.send(200, F("application/json"), stateJson());
}

void handleIP() {
    server.send(200, F("text/plain"), WiFi.localIP().toString());
}

// ── Setup & loop ──────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println(F("\nConnecting to WiFi..."));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());

    if (MDNS.begin("tictactoe")) {
        Serial.println(F("mDNS started — http://tictactoe.local"));
    }

    gameReset();

    server.on("/",      HTTP_GET,  handleRoot);
    server.on("/state", HTTP_GET,  handleState);
    server.on("/move",  HTTP_POST, handleMove);
    server.on("/reset", HTTP_POST, handleReset);
    server.on("/ip",    HTTP_GET,  handleIP);
    server.begin();
    Serial.println(F("HTTP server started"));
}

void loop() {
    server.handleClient();
}
