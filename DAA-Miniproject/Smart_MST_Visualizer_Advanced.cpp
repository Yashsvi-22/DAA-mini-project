#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

// ---------- Graph data ----------
struct Edge {
    int u, v, weight;
};

struct DSU {
    vector<int> parent, rankValue;

    DSU(int n) {
        parent.resize(n);
        rankValue.assign(n, 0);
        for (int i = 0; i < n; i++)
            parent[i] = i;
    }

    int findParent(int x) {
        if (parent[x] == x) return x;
        return parent[x] = findParent(parent[x]);
    }

    bool joinSets(int u, int v) {
        u = findParent(u);
        v = findParent(v);

        if (u == v) return false;

        if (rankValue[u] < rankValue[v])
            swap(u, v);

        parent[v] = u;

        if (rankValue[u] == rankValue[v])
            rankValue[u]++;

        return true;
    }
};

// ---------- Window controls ----------
HWND hVertices, hEdges, hStatus;
HWND hCost, hStats;
HWND hNext, hPrevious, hPlay;

const int ID_SAMPLE = 101;
const int ID_BUILD = 102;
const int ID_RUN = 103;
const int ID_NEXT = 104;
const int ID_PREVIOUS = 105;
const int ID_PLAY = 106;
const int ID_RESET = 107;
const int ID_CLEAR = 108;

const UINT TIMER_ID = 1;

const COLORREF BG = RGB(18, 24, 38);
const COLORREF PANEL = RGB(28, 36, 54);
const COLORREF WHITE = RGB(235, 241, 250);
const COLORREF MUTED = RGB(155, 170, 190);
const COLORREF BLUE = RGB(75, 140, 255);
const COLORREF GREEN = RGB(40, 210, 130);
const COLORREF RED = RGB(255, 90, 100);
const COLORREF ORANGE = RGB(255, 180, 65);
const COLORREF GREY = RGB(95, 108, 130);

vector<Edge> edges;
vector<int> order;
vector<bool> selected;

int vertexCount = 4;
int stepIndex = 0;
int currentMSTCost = 0;
int totalEdgeWeight = 0;
int selectedCount = 0;

bool graphReady = false;
bool algorithmFinished = false;
bool playing = false;

wstring statusText = L"Load the sample, then click Build Graph.";
wstring costText = L"MST cost: 0";
wstring statsText = L"Vertices: 4    Edges: 5    Selected: 0";

HFONT hFont, hSmallFont;

// ---------- Small UI helpers ----------
void setStatus(const wstring& s) {
    statusText = s;
    SetWindowTextW(hStatus, s.c_str());
}

void setCost(const wstring& s) {
    costText = s;
    SetWindowTextW(hCost, s.c_str());
}

void setStats(const wstring& s) {
    statsText = s;
    SetWindowTextW(hStats, s.c_str());
}

void addLabel(HWND parent, const wchar_t* text,
              int x, int y, int w, int h) {
    HWND label = CreateWindowW(
        L"STATIC", text, WS_CHILD | WS_VISIBLE,
        x, y, w, h, parent, nullptr, nullptr, nullptr
    );
    SendMessageW(label, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
}

HWND addButton(HWND parent, const wchar_t* text,
               int id, int x, int y, int w, int h) {
    HWND b = CreateWindowW(
        L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        x, y, w, h, parent, (HMENU)(INT_PTR)id,
        GetModuleHandleW(nullptr), nullptr
    );
    SendMessageW(b, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
    return b;
}

// ---------- Sample graph ----------
void loadSample() {
    vertexCount = 4;

    edges = {
        {0, 1, 2},
        {0, 2, 3},
        {1, 2, 1},
        {1, 3, 4},
        {2, 3, 5}
    };

    SetWindowTextW(hVertices, L"4");
    SetWindowTextW(
        hEdges,
        L"0 1 2\r\n0 2 3\r\n1 2 1\r\n1 3 4\r\n2 3 5"
    );

    graphReady = false;
    algorithmFinished = false;
    stepIndex = 0;
    currentMSTCost = 0;
    selectedCount = 0;
    playing = false;
    KillTimer(GetParent(hVertices), TIMER_ID);

    setStatus(L"Sample loaded. Click Build Graph.");
    setCost(L"MST cost: 0");
    setStats(L"Vertices: 4    Edges: 5    Selected: 0");

    InvalidateRect(GetParent(hVertices), nullptr, TRUE);
}

// ---------- Read graph input ----------
bool buildGraph() {
    wchar_t text[32] = {};
    GetWindowTextW(hVertices, text, 31);

    int n = _wtoi(text);

    if (n < 1 || n > 12) {
        setStatus(L"Enter between 1 and 12 vertices.");
        return false;
    }

    int length = GetWindowTextLengthW(hEdges);

    if (length == 0) {
        setStatus(L"Enter edges as: source destination weight.");
        return false;
    }

    wstring inputText(length + 1, L'\0');
    GetWindowTextW(hEdges, &inputText[0], length + 1);

    wistringstream input(inputText);
    vector<Edge> newEdges;

    int u, v, w;

    while (input >> u >> v >> w) {
        if (u < 0 || u >= n || v < 0 || v >= n) {
            setStatus(L"Invalid vertex. Use numbers from 0 to V-1.");
            return false;
        }

        newEdges.push_back({u, v, w});
    }

    if (newEdges.empty()) {
        setStatus(L"No valid edges found.");
        return false;
    }

    vertexCount = n;
    edges = newEdges;

    order.clear();
    selected.assign(edges.size(), false);

    for (int i = 0; i < (int)edges.size(); i++)
        order.push_back(i);

    sort(order.begin(), order.end(),
         [](int a, int b) {
             return edges[a].weight < edges[b].weight;
         });

    totalEdgeWeight = 0;
    for (const Edge& e : edges)
        totalEdgeWeight += e.weight;

    graphReady = true;
    algorithmFinished = false;
    stepIndex = 0;
    currentMSTCost = 0;
    selectedCount = 0;
    playing = false;

    KillTimer(GetParent(hVertices), TIMER_ID);

    setStatus(L"Graph built. Press Next Step or Play.");
    setCost(L"MST cost: 0");
    setStats(
        L"Vertices: " + to_wstring(vertexCount) +
        L"    Edges: " + to_wstring(edges.size()) +
        L"    Selected: 0"
    );

    InvalidateRect(GetParent(hVertices), nullptr, TRUE);
    return true;
}

// ---------- Kruskal step history ----------
struct StepRecord {
    int edgeIndex;
    bool accepted;
    int costAfter;
    int selectedAfter;
    wstring explanation;
};

vector<StepRecord> history;
int visibleSteps = 0;

void prepareAlgorithm() {
    history.clear();
    selected.assign(edges.size(), false);

    DSU dsu(vertexCount);
    int cost = 0;
    int count = 0;

    for (int idx : order) {
        const Edge& e = edges[idx];

        bool accepted = dsu.joinSets(e.u, e.v);

        if (accepted) {
            cost += e.weight;
            count++;
        }

        wstring explanation;

        if (accepted) {
            explanation = L"Edge " + to_wstring(e.u) + L"-" +
                to_wstring(e.v) + L" is selected. No cycle is formed.";
        } else {
            explanation = L"Edge " + to_wstring(e.u) + L"-" +
                to_wstring(e.v) + L" is rejected because it forms a cycle.";
        }

        history.push_back({
            idx, accepted, cost, count, explanation
        });

        if (count == vertexCount - 1)
            break;
    }

    visibleSteps = 0;
    currentMSTCost = 0;
    selectedCount = 0;
    algorithmFinished = false;
}

void updateView() {
    fill(selected.begin(), selected.end(), false);

    currentMSTCost = 0;
    selectedCount = 0;

    for (int i = 0; i < visibleSteps; i++) {
        const StepRecord& s = history[i];

        if (s.accepted) {
            selected[s.edgeIndex] = true;
            currentMSTCost += edges[s.edgeIndex].weight;
            selectedCount++;
        }
    }

    algorithmFinished = visibleSteps >= (int)history.size();

    if (visibleSteps == 0) {
        setStatus(L"Ready. Press Next Step or Play to begin.");
    } else {
        setStatus(history[visibleSteps - 1].explanation);
    }

    if (algorithmFinished) {
        if (selectedCount == vertexCount - 1) {
            setStatus(L"Complete! The green edges form the MST.");
        } else {
            setStatus(L"Complete, but the graph is disconnected.");
        }
    }

    int reduction = totalEdgeWeight - currentMSTCost;

    setCost(
        L"MST cost: " + to_wstring(currentMSTCost) +
        L"    All edge weights: " + to_wstring(totalEdgeWeight) +
        L"    Difference: " + to_wstring(reduction)
    );

    setStats(
        L"Vertices: " + to_wstring(vertexCount) +
        L"    Edges: " + to_wstring(edges.size()) +
        L"    Selected: " + to_wstring(selectedCount) +
        L" / " + to_wstring(max(0, vertexCount - 1)) +
        L"    Step: " + to_wstring(visibleSteps) +
        L" / " + to_wstring(history.size())
    );

    InvalidateRect(GetParent(hVertices), nullptr, TRUE);
}

void nextStep() {
    if (!graphReady) {
        setStatus(L"Click Build Graph first.");
        return;
    }

    if (history.empty())
        prepareAlgorithm();

    if (visibleSteps < (int)history.size())
        visibleSteps++;

    updateView();
}

void previousStep() {
    if (!graphReady) return;

    playing = false;
    KillTimer(GetParent(hVertices), TIMER_ID);
    SetWindowTextW(hPlay, L"Play");

    if (visibleSteps > 0)
        visibleSteps--;

    updateView();
}

void runAll() {
    if (!graphReady) {
        setStatus(L"Click Build Graph first.");
        return;
    }

    if (history.empty())
        prepareAlgorithm();

    visibleSteps = (int)history.size();
    playing = false;
    KillTimer(GetParent(hVertices), TIMER_ID);
    SetWindowTextW(hPlay, L"Play");

    updateView();
}

void resetSteps() {
    playing = false;
    KillTimer(GetParent(hVertices), TIMER_ID);
    SetWindowTextW(hPlay, L"Play");

    visibleSteps = 0;
    currentMSTCost = 0;
    selectedCount = 0;
    algorithmFinished = false;

    fill(selected.begin(), selected.end(), false);
    updateView();
}

// ---------- Drawing helpers ----------
void drawText(HDC dc, int x, int y, const wstring& s,
              COLORREF color, HFONT font = nullptr) {
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);

    HGDIOBJ old = nullptr;

    if (font)
        old = SelectObject(dc, font);

    TextOutW(dc, x, y, s.c_str(), (int)s.size());

    if (old)
        SelectObject(dc, old);
}

void fillRectColor(HDC dc, int x, int y, int w, int h,
                   COLORREF color) {
    RECT r = {x, y, x + w, y + h};
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &r, brush);
    DeleteObject(brush);
}

void drawGraph(HDC dc, RECT client) {
    int left = 410;
    int top = 95;
    int right = client.right - 25;
    int bottom = (int)min(client.bottom - 25L, 495L);
    if (right <= left || bottom <= top) return;

    fillRectColor(dc, left, top, right - left,
                  bottom - top, PANEL);

    drawText(dc, left + 20, top + 15,
             L"GRAPH VISUALIZATION", WHITE, hFont);

    if (vertexCount <= 0) return;

    int cx = (left + right) / 2;
    int cy = (top + bottom) / 2 + 10;

    int radius = min((right - left) / 2 - 60,
                     (bottom - top) / 2 - 60);

    radius = max(radius, 35);

    vector<POINT> points(vertexCount);

    const double pi = 3.14159265358979323846;

    for (int i = 0; i < vertexCount; i++) {
        double angle = -pi / 2 +
            2.0 * pi * i / vertexCount;

        points[i].x = cx + (int)(radius * cos(angle));
        points[i].y = cy + (int)(radius * sin(angle));
    }

    // Draw edges.
    for (int i = 0; i < (int)edges.size(); i++) {
        const Edge& e = edges[i];

        if (e.u >= vertexCount || e.v >= vertexCount)
            continue;

        COLORREF color = GREY;
        int thickness = 2;

        if (i < (int)selected.size() && selected[i]) {
            color = GREEN;
            thickness = 4;
        }

        HPEN pen = CreatePen(PS_SOLID, thickness, color);
        HGDIOBJ oldPen = SelectObject(dc, pen);

        MoveToEx(dc, points[e.u].x, points[e.u].y, nullptr);
        LineTo(dc, points[e.v].x, points[e.v].y);

        SelectObject(dc, oldPen);
        DeleteObject(pen);

        int mx = (points[e.u].x + points[e.v].x) / 2;
        int my = (points[e.u].y + points[e.v].y) / 2;

        fillRectColor(dc, mx - 18, my - 11, 36, 22, PANEL);

        drawText(dc, mx - 9, my - 8,
                 to_wstring(e.weight), ORANGE, hSmallFont);
    }

    // Draw vertices.
    for (int i = 0; i < vertexCount; i++) {
        HBRUSH brush = CreateSolidBrush(BLUE);
        HPEN pen = CreatePen(PS_SOLID, 2, WHITE);

        HGDIOBJ oldBrush = SelectObject(dc, brush);
        HGDIOBJ oldPen = SelectObject(dc, pen);

        Ellipse(dc, points[i].x - 20, points[i].y - 20,
                points[i].x + 20, points[i].y + 20);

        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);

        DeleteObject(brush);
        DeleteObject(pen);

        wstring label = to_wstring(i);

        SIZE size{};
        GetTextExtentPoint32W(
            dc, label.c_str(), (int)label.size(), &size
        );

        drawText(dc,
            points[i].x - size.cx / 2,
            points[i].y - size.cy / 2,
            label, WHITE);
    }

    drawText(dc, left + 20, bottom - 28,
             L"GREEN = selected MST edge", GREEN, hSmallFont);
}

// ---------- Main window ----------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
                            WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        hFont = CreateFontW(
            20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI"
        );

        hSmallFont = CreateFontW(
            15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI"
        );

        addLabel(hwnd, L"SMART MST VISUALIZER", 20, 15, 360, 30);
        addLabel(hwnd, L"KRUSKAL'S ALGORITHM", 20, 43, 300, 22);

        addLabel(hwnd, L"Vertices (0 to V-1)", 20, 75, 220, 20);

        hVertices = CreateWindowW(
            L"EDIT", L"4",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            20, 98, 100, 28, hwnd,
            (HMENU)110, nullptr, nullptr
        );

        addLabel(hwnd, L"Edges: source destination weight",
                 20, 137, 350, 22);

        hEdges = CreateWindowW(
            L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            20, 162, 355, 200, hwnd,
            (HMENU)111, nullptr, nullptr
        );

        addButton(hwnd, L"Load Sample", ID_SAMPLE, 20, 375, 110, 32);
        addButton(hwnd, L"Build Graph", ID_BUILD, 140, 375, 110, 32);
        addButton(hwnd, L"Run All", ID_RUN, 260, 375, 100, 32);

        hPrevious = addButton(hwnd, L"Previous", ID_PREVIOUS,
                              20, 420, 100, 32);

        hNext = addButton(hwnd, L"Next Step", ID_NEXT,
                          130, 420, 100, 32);

        hPlay = addButton(hwnd, L"Play", ID_PLAY,
                          240, 420, 80, 32);

        addButton(hwnd, L"Reset Steps", ID_RESET,
                  20, 462, 120, 32);

        addButton(hwnd, L"Clear", ID_CLEAR,
                  150, 462, 80, 32);

        hCost = CreateWindowW(
            L"STATIC", L"MST cost: 0",
            WS_CHILD | WS_VISIBLE,
            20, 510, 900, 24, hwnd, nullptr, nullptr, nullptr
        );

        hStats = CreateWindowW(
            L"STATIC", L"Vertices: 4",
            WS_CHILD | WS_VISIBLE,
            20, 540, 900, 24, hwnd, nullptr, nullptr, nullptr
        );

        hStatus = CreateWindowW(
            L"STATIC", L"Load sample to begin.",
            WS_CHILD | WS_VISIBLE,
            20, 570, 950, 30, hwnd, nullptr, nullptr, nullptr
        );

        HWND controls[] = {
            hVertices, hEdges, hCost, hStats, hStatus
        };

        for (HWND control : controls)
            SendMessageW(control, WM_SETFONT,
                         (WPARAM)hSmallFont, TRUE);

        loadSample();
        return 0;
    }

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, WHITE);
        SetBkColor(hdc, PANEL);
        static HBRUSH editBrush = CreateSolidBrush(PANEL);
        return (LRESULT)editBrush;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, WHITE);
        SetBkColor(hdc, BG);
        static HBRUSH staticBrush = CreateSolidBrush(BG);
        return (LRESULT)staticBrush;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lp;
        if (dis && dis->CtlType == ODT_BUTTON) {
            bool pressed = (dis->itemState & ODS_SELECTED) != 0;
            bool focused = (dis->itemState & ODS_FOCUS) != 0;
            COLORREF buttonColor = pressed ? RGB(45, 95, 190) : RGB(39, 70, 115);
            HBRUSH brush = CreateSolidBrush(buttonColor);
            FillRect(dis->hDC, &dis->rcItem, brush);
            DeleteObject(brush);

            HPEN pen = CreatePen(PS_SOLID, 1, BLUE);
            HGDIOBJ oldPen = SelectObject(dis->hDC, pen);
            HGDIOBJ oldBrush = SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
            Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top,
                      dis->rcItem.right, dis->rcItem.bottom);
            SelectObject(dis->hDC, oldBrush);
            SelectObject(dis->hDC, oldPen);
            DeleteObject(pen);

            wchar_t caption[128] = {};
            GetWindowTextW(dis->hwndItem, caption, 127);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, WHITE);
            HFONT oldFont = (HFONT)SelectObject(dis->hDC, hSmallFont);
            DrawTextW(dis->hDC, caption, -1, &dis->rcItem,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(dis->hDC, oldFont);

            if (focused) DrawFocusRect(dis->hDC, &dis->rcItem);
            return TRUE;
        }
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case ID_SAMPLE:
            loadSample();
            return 0;

        case ID_BUILD:
            buildGraph();
            if (graphReady) {
                prepareAlgorithm();
                updateView();
            }
            return 0;

        case ID_RUN:
            runAll();
            return 0;

        case ID_NEXT:
            nextStep();
            return 0;

        case ID_PREVIOUS:
            previousStep();
            return 0;

        case ID_RESET:
            resetSteps();
            return 0;

        case ID_PLAY:
            if (!graphReady) {
                setStatus(L"Build the graph first.");
                return 0;
            }

            if (playing) {
                playing = false;
                KillTimer(hwnd, TIMER_ID);
                SetWindowTextW(hPlay, L"Play");
            } else {
                if (history.empty())
                    prepareAlgorithm();

                if (visibleSteps >= (int)history.size())
                    visibleSteps = 0;

                playing = true;
                SetWindowTextW(hPlay, L"Pause");
                SetTimer(hwnd, TIMER_ID, 800, nullptr);
            }
            return 0;

        case ID_CLEAR:
            playing = false;
            KillTimer(hwnd, TIMER_ID);
            graphReady = false;
            edges.clear();
            order.clear();
            selected.clear();
            history.clear();
            vertexCount = 0;
            currentMSTCost = 0;
            selectedCount = 0;
            SetWindowTextW(hVertices, L"");
            SetWindowTextW(hEdges, L"");
            setStatus(L"Cleared. Enter a graph and click Build Graph.");
            setCost(L"MST cost: 0");
            setStats(L"No graph loaded.");
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        break;

    case WM_TIMER:
        if (wp == TIMER_ID && playing) {
            if (visibleSteps < (int)history.size()) {
                visibleSteps++;
                updateView();
            }

            if (visibleSteps >= (int)history.size()) {
                playing = false;
                KillTimer(hwnd, TIMER_ID);
                SetWindowTextW(hPlay, L"Play");
            }
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);

        RECT client;
        GetClientRect(hwnd, &client);

        fillRectColor(dc, 0, 0,
                      client.right, client.bottom, BG);

        drawGraph(dc, client);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);

        DeleteObject(hFont);
        DeleteObject(hSmallFont);

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ---------- Program entry point ----------
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE,
                    PWSTR, int showCommand) {
    INITCOMMONCONTROLSEX controls = {
        sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES
    };
    InitCommonControlsEx(&controls);

    const wchar_t CLASS_NAME[] = L"SmartMSTVisualizerWindow";

    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassW(&wc)) {
        MessageBoxW(nullptr, L"Window registration failed.",
                    L"Smart MST Visualizer", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME,
        L"Smart MST Visualizer - Kruskal's Algorithm",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1150, 700,
        nullptr, nullptr, instance, nullptr
    );

    if (!hwnd) {
        MessageBoxW(nullptr, L"Could not create the window.",
                    L"Smart MST Visualizer", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG message{};

    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return (int)message.wParam;
}
