#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0601
#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <exdisp.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>

#define MX 64
#define PL 1024

typedef struct {
  WCHAR ex[PL];
  WCHAR ar[PL];
  WCHAR wd[PL];
  WCHAR mx[256];
  UINT md;
  UINT vk;
  DWORD pid;
  int dr;
} It;

static It its[MX];
static int ni;
static UINT qm;
static UINT qv;

typedef struct {
  const char *n;
  UINT v;
} Kn;

static const Kn kt[] = {
  {"space", VK_SPACE}, {"enter", VK_RETURN}, {"esc", VK_ESCAPE},
  {"tab", VK_TAB}, {"up", VK_UP}, {"down", VK_DOWN}, {"left", VK_LEFT},
  {"right", VK_RIGHT}, {"home", VK_HOME}, {"end", VK_END},
  {"pgup", VK_PRIOR}, {"pgdn", VK_NEXT}, {"ins", VK_INSERT},
  {"del", VK_DELETE}, {"back", VK_BACK}, {"pause", VK_PAUSE},
  {"backspace", VK_BACK}, {"equals", VK_OEM_PLUS}, {"minus", VK_OEM_MINUS},
  {"print", VK_SNAPSHOT}, {"scroll", VK_SCROLL}, {"caps", VK_CAPITAL},
  {NULL, 0}
};

static UINT kc(const char *t) {
  int n = (int)strlen(t);
  int i;
  if (n == 1) {
    if (t[0] >= 'a' && t[0] <= 'z') return (UINT)toupper((unsigned char)t[0]);
    if (t[0] >= '0' && t[0] <= '9') return (UINT)t[0];
    switch (t[0]) {
      case '=': return VK_OEM_PLUS;
      case '-': return VK_OEM_MINUS;
      case ',': return VK_OEM_COMMA;
      case '.': return VK_OEM_PERIOD;
      case ';': return VK_OEM_1;
      case '/': return VK_OEM_2;
      case '`': return VK_OEM_3;
      case '[': return VK_OEM_4;
      case '\\': return VK_OEM_5;
      case ']': return VK_OEM_6;
      case '\'': return VK_OEM_7;
    }
  }
  if (t[0] == 'f' && n > 1 && isdigit((unsigned char)t[1])) {
    i = atoi(t + 1);
    if (i >= 1 && i <= 24) return VK_F1 + i - 1;
  }
  if (!strncmp(t, "num", 3) && n == 4 && isdigit((unsigned char)t[3]))
    return VK_NUMPAD0 + (t[3] - '0');
  for (i = 0; kt[i].n; i++)
    if (!strcmp(t, kt[i].n)) return kt[i].v;
  return 0;
}

static int pb(const char *s, UINT *m, UINT *v) {
  char t[32];
  int i;
  const char *p = s;
  *m = 0;
  *v = 0;
  for (;;) {
    i = 0;
    while (*p && *p != '+') {
      if (*p != ' ' && i < 31) t[i++] = (char)tolower((unsigned char)*p);
      p++;
    }
    t[i] = 0;
    if (i) {
      if (!strcmp(t, "ctrl") || !strcmp(t, "control")) *m |= MOD_CONTROL;
      else if (!strcmp(t, "alt")) *m |= MOD_ALT;
      else if (!strcmp(t, "shift")) *m |= MOD_SHIFT;
      else if (!strcmp(t, "win") || !strcmp(t, "super")) *m |= MOD_WIN;
      else *v = kc(t);
    }
    if (!*p) break;
    p++;
  }
  return *v != 0;
}

static int gs(const char *s, const char *k, char *o, int n) {
  char p[64];
  const char *q;
  int i = 0;
  char c;
  snprintf(p, sizeof p, "\"%s\"", k);
  q = strstr(s, p);
  if (!q) return 0;
  q += strlen(p);
  while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') q++;
  if (*q != ':') return 0;
  q++;
  while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') q++;
  if (*q != '"') return 0;
  q++;
  while (*q && *q != '"') {
    c = *q++;
    if (c == '\\' && *q) {
      c = *q++;
      if (c == 'n') c = '\n';
      else if (c == 't') c = '\t';
    }
    if (i < n - 1) o[i++] = c;
  }
  o[i] = 0;
  return 1;
}

static char *ob(char *a) {
  int d = 0;
  int s = 0;
  for (; *a; a++) {
    if (s) {
      if (*a == '\\' && a[1]) a++;
      else if (*a == '"') s = 0;
      continue;
    }
    if (*a == '"') s = 1;
    else if (*a == '{') d++;
    else if (*a == '}') {
      d--;
      if (!d) return a;
    }
  }
  return NULL;
}

static void u2w(const char *s, WCHAR *o, int n) {
  if (!MultiByteToWideChar(CP_UTF8, 0, s, -1, o, n)) o[0] = 0;
}

static void ai(char *a) {
  char b[PL * 3];
  char t[PL];
  WCHAR w[PL];
  UINT m, v;
  It *i;
  DWORD a2;
  if (ni >= MX) return;
  i = &its[ni];
  memset(i, 0, sizeof *i);
  if (!gs(a, "exe", b, sizeof b)) return;
  if (!gs(a, "bind", t, sizeof t)) return;
  if (!pb(t, &m, &v)) return;
  u2w(b, w, PL);
  if (!GetFullPathNameW(w, PL, i->ex, NULL)) return;
  a2 = GetFileAttributesW(i->ex);
  i->dr = a2 != INVALID_FILE_ATTRIBUTES && (a2 & FILE_ATTRIBUTE_DIRECTORY);
  if (gs(a, "args", b, sizeof b)) u2w(b, i->ar, PL);
  if (gs(a, "dir", b, sizeof b)) u2w(b, i->wd, PL);
  if (gs(a, "mutex", b, sizeof b)) u2w(b, i->mx, 256);
  i->md = m;
  i->vk = v;
  ni++;
}

static void ld(const WCHAR *f) {
  FILE *h = _wfopen(f, L"rb");
  long n;
  char *b, *s, *a, *e;
  char q[64];
  if (!h) return;
  fseek(h, 0, SEEK_END);
  n = ftell(h);
  fseek(h, 0, SEEK_SET);
  b = (char *)malloc(n + 1);
  if (!b) {
    fclose(h);
    return;
  }
  n = (long)fread(b, 1, n, h);
  b[n] = 0;
  fclose(h);
  s = b;
  if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB && (unsigned char)s[2] == 0xBF) s += 3;
  if (gs(s, "quit", q, sizeof q)) pb(q, &qm, &qv);
  a = strchr(s, '[');
  if (a) {
    a++;
    for (;;) {
      while (*a && *a != '{' && *a != ']') a++;
      if (*a != '{') break;
      e = ob(a);
      if (!e) break;
      *e = 0;
      ai(a);
      *e = '}';
      a = e + 1;
    }
  }
  free(b);
}

static int al(DWORD id) {
  HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, id);
  int r;
  if (!h) return 0;
  r = WaitForSingleObject(h, 0) == WAIT_TIMEOUT;
  CloseHandle(h);
  return r;
}

static int mt(const It *i) {
  HANDLE h;
  if (!i->mx[0]) return 0;
  h = OpenMutexW(SYNCHRONIZE, FALSE, i->mx);
  if (h) {
    CloseHandle(h);
    return 1;
  }
  return GetLastError() == ERROR_ACCESS_DENIED;
}

static int ffp(const WCHAR *f, DWORD *o, int n) {
  int c = 0;
  HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  PROCESSENTRY32W e;
  HANDLE p;
  WCHAR b[PL];
  DWORD l;
  if (h == INVALID_HANDLE_VALUE) return 0;
  e.dwSize = sizeof e;
  if (Process32FirstW(h, &e)) {
    do {
      p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, e.th32ProcessID);
      if (p) {
        l = PL;
        if (QueryFullProcessImageNameW(p, 0, b, &l) && !_wcsicmp(b, f) && c < n)
          o[c++] = e.th32ProcessID;
        CloseHandle(p);
      }
    } while (Process32NextW(h, &e));
  }
  CloseHandle(h);
  return c;
}

static BOOL CALLBACK ew(HWND w, LPARAM l) {
  DWORD p = 0;
  GetWindowThreadProcessId(w, &p);
  if (p == (DWORD)l) PostMessageW(w, WM_CLOSE, 0, 0);
  return TRUE;
}

static void kl(DWORD id) {
  HANDLE h = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, id);
  if (!h) return;
  EnumWindows(ew, (LPARAM)id);
  if (WaitForSingleObject(h, 3000) != WAIT_OBJECT_0) TerminateProcess(h, 0);
  CloseHandle(h);
}

static void ln(It *i) {
  WCHAR c[PL * 2 + 8];
  WCHAR d[PL];
  WCHAR *b;
  STARTUPINFOW s;
  PROCESS_INFORMATION pi;
  wcscpy(c, L"\"");
  wcscat(c, i->ex);
  wcscat(c, L"\"");
  if (i->ar[0]) {
    wcscat(c, L" ");
    wcscat(c, i->ar);
  }
  if (i->wd[0]) wcscpy(d, i->wd);
  else {
    wcscpy(d, i->ex);
    b = wcsrchr(d, L'\\');
    if (b) *b = 0;
  }
  memset(&s, 0, sizeof s);
  s.cb = sizeof s;
  memset(&pi, 0, sizeof pi);
  if (CreateProcessW(NULL, c, NULL, NULL, FALSE, 0, NULL, d, &s, &pi)) {
    i->pid = pi.dwProcessId;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
}

static void tsl(WCHAR *s) {
  size_t n = wcslen(s);
  while (n > 3 && s[n - 1] == L'\\') s[--n] = 0;
}

static int fw(const WCHAR *f, HWND *o, int n) {
  IShellWindows *sw = NULL;
  long c = 0, k;
  int r = 0;
  VARIANT v;
  IDispatch *d;
  IWebBrowser2 *b;
  BSTR u;
  WCHAR p[PL];
  DWORD l;
  SHANDLE_PTR h;
  if (FAILED(CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void **)&sw))) return 0;
  IShellWindows_get_Count(sw, &c);
  for (k = 0; k < c && r < n; k++) {
    d = NULL;
    b = NULL;
    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = k;
    if (SUCCEEDED(IShellWindows_Item(sw, v, &d)) && d) {
      if (SUCCEEDED(IDispatch_QueryInterface(d, &IID_IWebBrowser2, (void **)&b)) && b) {
        u = NULL;
        if (SUCCEEDED(IWebBrowser2_get_LocationURL(b, &u)) && u) {
          l = PL;
          if (SUCCEEDED(PathCreateFromUrlW(u, p, &l, 0))) {
            tsl(p);
            if (!_wcsicmp(p, f)) {
              h = 0;
              IWebBrowser2_get_HWND(b, &h);
              if (h) o[r++] = (HWND)h;
            }
          }
          SysFreeString(u);
        }
        IWebBrowser2_Release(b);
      }
      IDispatch_Release(d);
    }
  }
  IShellWindows_Release(sw);
  return r;
}

static void td(It *i) {
  HWND w[16];
  WCHAR f[PL];
  int n, k;
  wcscpy(f, i->ex);
  tsl(f);
  n = fw(f, w, 16);
  if (n > 0) {
    for (k = 0; k < n; k++) PostMessageW(w[k], WM_CLOSE, 0, 0);
    return;
  }
  ShellExecuteW(NULL, L"open", i->ex, NULL, NULL, SW_SHOWNORMAL);
}

static void tg(It *i) {
  DWORD p[MX];
  int n, k, f = 0;
  if (i->dr) {
    td(i);
    return;
  }
  n = ffp(i->ex, p, MX - 1);
  if (i->pid && al(i->pid)) {
    for (k = 0; k < n; k++)
      if (p[k] == i->pid) f = 1;
    if (!f) p[n++] = i->pid;
  }
  if (n > 0) {
    for (k = 0; k < n; k++) kl(p[k]);
    i->pid = 0;
    return;
  }
  if (mt(i)) return;
  ln(i);
}

static void as(void) {
  HKEY k;
  WCHAR p[PL];
  WCHAR v[PL + 4];
  GetModuleFileNameW(NULL, p, PL);
  wcscpy(v, L"\"");
  wcscat(v, p);
  wcscat(v, L"\"");
  if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, NULL, 0, KEY_SET_VALUE, NULL, &k, NULL) == ERROR_SUCCESS) {
    RegSetValueExW(k, L"hkl", 0, REG_SZ, (const BYTE *)v, (DWORD)((wcslen(v) + 1) * sizeof(WCHAR)));
    RegCloseKey(k);
  }
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE hp, LPSTR cl, int sw) {
  HANDLE m;
  WCHAR f[PL];
  WCHAR *b;
  MSG g;
  int i, k;
  (void)h;
  (void)hp;
  (void)cl;
  (void)sw;
  m = CreateMutexW(NULL, TRUE, L"hkl_single");
  if (!m || GetLastError() == ERROR_ALREADY_EXISTS) return 0;
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  as();
  GetModuleFileNameW(NULL, f, PL);
  b = wcsrchr(f, L'\\');
  if (b) *b = 0;
  wcscat(f, L"\\hkl.json");
  ld(f);
  for (i = 0; i < ni; i++)
    RegisterHotKey(NULL, i + 1, its[i].md | MOD_NOREPEAT, its[i].vk);
  if (qv) RegisterHotKey(NULL, MX + 1, qm | MOD_NOREPEAT, qv);
  while (GetMessageW(&g, NULL, 0, 0) > 0) {
    if (g.message == WM_HOTKEY) {
      k = (int)g.wParam;
      if (k == MX + 1) break;
      if (k >= 1 && k <= ni) tg(&its[k - 1]);
    }
  }
  return 0;
}
