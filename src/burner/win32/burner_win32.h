#ifdef __GNUC__
// silence warnings with GCC 4.6.1
#undef _WIN32_WINDOWS
#undef _WIN32_IE
#undef _WIN32_WINNT
#undef WINVER
#endif

#define _WIN32_WINDOWS 0x0410
//#define _WIN32_WINNT 0x0400
#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#define STRICT

#if defined (_UNICODE)
#ifndef UNICODE
 #define UNICODE
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>

#include <mmsystem.h>
#include <shellapi.h>

// fix for shlwapi.h
#ifndef _In_
#define _In_
#endif

#include <shlwapi.h>
#include "d3dkmt_sync.h"

INT32 DSCore_Init();
INT32 DICore_Init();
INT32 DDCore_Init();
INT32 Dx9Core_Init();

// Additions to the Cygwin/MinGW win32 headers
#ifdef __GNUC__
 #include "mingw_win32.h"
#endif

#include "resource.h"
#include "resource_string.h"
#include "net.h"
// ---------------------------------------------------------------------------

// Macro for releasing a COM object
#define RELEASE(x) { if ((x)) (x)->Release(); (x) = NULL; }

#define KEY_DOWN(Code) ((GetAsyncKeyState(Code) & 0x8000) ? 1 : 0)

// Macros used for handling Window Messages
#define HANDLE_WM_ENTERMENULOOP(hwnd, wParam, lParam, fn)		\
	((fn)((hwnd), (BOOL)(wParam)), 0L)

#ifdef __GNUC__
#define HANDLE_WM_EXITMENULOOP(hwnd, wParam, lParam, fn)		\
	((fn)((hwnd), (BOOL)(wParam)))
#else
#define HANDLE_WM_EXITMENULOOP(hwnd, wParam, lParam, fn)		\
	((fn)((hwnd), (BOOL)(wParam)), 0L)
#endif

#define HANDLE_WM_ENTERSIZEMOVE(hwnd, wParam, lParam, fn)		\
	((fn)(hwnd), 0L)

#define HANDLE_WM_EXITSIZEMOVE(hwnd, wParam, lParam, fn)		\
	((fn)(hwnd), 0L)

#define HANDLE_WM_UNINITMENUPOPUP(hwnd,wParam,lParam,fn)		\
	((fn)((hwnd), (HMENU)(wParam), (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)),0)

// Extra macro used for handling Window Messages
#define HANDLE_MSGB(hwnd, message, fn)							\
	case (message): 												\
		HANDLE_##message((hwnd), (wParam), (lParam), (fn)); \
		break;

// Macro used for re-initialiging video/sound/input
// #define POST_INITIALISE_MESSAGE { dprintf(_T("*** (re-) initialising - %s %i\n"), _T(__FILE__), __LINE__); PostMessage(NULL, WM_APP + 0, 0, 0); }
#define POST_INITIALISE_MESSAGE PostMessage(NULL, WM_APP + 0, 0, 0)

// ---------------------------------------------------------------------------

// main.cpp
#if defined (FBNEO_DEBUG)
extern bool bDisableDebugConsole;                   // Disable debug console?
#endif
extern HINSTANCE hAppInst;							// Application Instance
extern HANDLE hMainThread;							// Handle to the main thread
extern long int nMainThreadID;						// ID of the main thread
extern int nAppProcessPriority;
extern int nAppShowCmd;

extern HACCEL hAccel;

extern int nAppVirtualFps;							// virtual fps

#define EXE_NAME_SIZE (32)
extern TCHAR szAppExeName[EXE_NAME_SIZE + 1];
extern TCHAR szAppBurnVer[16];

extern int  nCmdOptUsed;
extern bool bAlwaysProcessKeyboardInput;
extern bool bAlwaysCreateSupportFolders;

extern bool bAutoLoadGameList;

extern bool bQuietLoading;
extern bool bNoPopups;
extern bool bShonkyProfileMode;

extern bool bNoChangeNumLock;
extern bool bMonitorAutoCheck;

// Used for the load/save dialog in commdlg.h
extern TCHAR szChoice[MAX_PATH];					// File chosen by the user
extern OPENFILENAME ofn;

// Used to convert strings when possibly needed
/* const */ char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int nOutSize);
/* const */ TCHAR* ANSIToTCHAR(const char* pszString, TCHAR* pszOutString, int nOutSize);

CHAR *astring_from_utf8(const char *s);
char *utf8_from_astring(const CHAR *s);

WCHAR *wstring_from_utf8(const char *s);
char *utf8_from_wstring(const WCHAR *s);
void tcharstrreplace(TCHAR *pszSRBuffer, const TCHAR *pszFind, const TCHAR *pszReplace);

#ifdef _UNICODE
#define tstring_from_utf8 wstring_from_utf8
#define utf8_from_tstring utf8_from_wstring
#else // !_UNICODE
#define tstring_from_utf8 astring_from_utf8
#define utf8_from_tstring utf8_from_astring
#endif // _UNICODE

int dprintf(TCHAR* pszFormat, ...);					// Use instead of printf() in the UI

void MonitorAutoCheck();

void AppCleanup();
int AppMessage(MSG* pMsg);
bool AppProcessKeyboardInput();

// localise.cpp
extern bool bLocalisationActive;
extern TCHAR szLocalisationTemplate[MAX_PATH];

void FBALocaliseExit();
int FBALocaliseInit(TCHAR* pszTemplate);
int FBALocaliseLoadTemplate();
int FBALocaliseCreateTemplate();
HMENU FBALoadMenu(HINSTANCE hInstance, LPTSTR lpMenuName);
INT_PTR FBADialogBox(HINSTANCE hInstance, LPTSTR lpTemplate, HWND hWndParent, DLGPROC  lpDialogFunc);
HWND FBACreateDialog(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc);
int FBALoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);
TCHAR* FBALoadStringEx(HINSTANCE hInstance, UINT uID, bool bTranslate);

// localise_gamelist.cpp
extern TCHAR szGamelistLocalisationTemplate[MAX_PATH];
extern bool nGamelistLocalisationActive;

void BurnerDoGameListLocalisation();
void BurnerExitGameListLocalisation();
int FBALocaliseGamelistLoadTemplate();
int FBALocaliseGamelistCreateTemplate();

INT32 BurnDrvSetFullNameW(TCHAR* szName, INT32 i = nBurnDrvActive);

// popup_win32.cpp
enum FBAPopupType { MT_NONE = 0, MT_ERROR, MT_WARNING, MT_INFO };

#define PUF_TYPE_ERROR			(1)
#define PUF_TYPE_WARNING		(2)
#define PUF_TYPE_INFO			(3)
#define PUF_TYPE_LOGONLY		(8)

#define PUF_TEXT_TRANSLATE		(1 << 16)

#define PUF_TEXT_NO_TRANSLATE	(0)
#define PUF_TEXT_DEFAULT		(PUF_TEXT_TRANSLATE)

int FBAPopupDisplay(int nFlags);
int FBAPopupAddText(int nFlags, TCHAR* pszFormat, ...);
int FBAPopupDestroyText();

// sysinfo.cpp
LONG CALLBACK ExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo);
int SystemInfoCreate();

// splash.cpp
int SplashCreate();
void SplashDestroy(bool bForce);

extern int nSplashTime;

// about.cpp
int AboutCreate();
int FirstUsageCreate();

// media.cpp
int MediaInit();
int MediaExit();

// misc_win32.cpp
extern bool bEnableHighResTimer;
extern bool bIsWindowsXPorGreater;
extern bool bIsWindowsXP;
BOOL DetectWindowsVersion();
int AppDirectory();
void UpdatePath(TCHAR* path);
void RegisterExtensions(bool bCreateKeys);
int GetClientScreenRect(HWND hWnd, RECT* pRect);
int WndInMid(HWND hMid, HWND hBase);
char* DecorateGameName(unsigned int nBurnDrv);
void EnableHighResolutionTiming();
void DisableHighResolutionTiming();

// drv.cpp
extern int bDrvOkay;								// 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions
extern TCHAR szAppRomPaths[DIRS_MAX][MAX_PATH];
extern TCHAR szAppQuickPath[MAX_PATH];
int DrvInit(int nDrvNum, bool bRestore);
int DrvInitCallback();								// Used when Burn library needs to load a game. DrvInit(nBurnSelect, false)
int DrvExit();
void NeoCDZRateChangeback();

// burn_shift
extern INT32 BurnShiftEnabled;
// burn_gun
extern bool bBurnGunDrawReticles;

// run.cpp
extern int bRunPause;
extern int bAltPause;
extern int bAlwaysDrawFrames;
extern int nSlowMo;
//extern INT32 bRunAhead;  // in burn.h! (partially platform agnostic feature)
extern int kNetGame;
extern int k_player_id;
extern int k_numplayers;
extern int k_bLoadNetgame;
extern char k_game_str[255];

int RunIdle();
int RunFrame(int bDraw, int bPause);
int RunMessageLoop();
int RunReset();
void ToggleLayer(unsigned char thisLayer);

// scrn.cpp
extern HWND hScrnWnd;								// Handle to the screen window
extern HWND hRebar;									// Handle to the Rebar control containing the menu
extern HWND hwndChat;
extern bool bRescanRoms;
extern bool bMenuEnabled;
extern INT32 nQuickOpen;

extern RECT SystemWorkArea;							// The full screen area
extern int nWindowPosX, nWindowPosY;

extern int nSavestateSlot;

int ScrnInit();
int ScrnExit();
int ScrnSize();
int ScrnTitle();
void SetPauseMode(bool bPause);
int ActivateChat();
void DeActivateChat();
int BurnerLoadDriver(TCHAR *pszDriverName);
int StartFromReset(TCHAR *szDriverName, bool bLoadSram);
void PausedRedraw(void);
INT32 is_netgame_or_recording();
void ScrnInitLua();
void ScrnExitLua();
char* DecorateKailleraGameName(UINT32 nBurnDrv);
INT32 CreateAllDatfilesWindows(bool bSilent = false, const TCHAR* pszSpecDir = NULL);
INT32 RomDataLoadDriver(const TCHAR* pszSelDat);
INT32 BurnerQuickLoad(const INT32 nMode, const TCHAR* pszSelect);

// bzip.cpp
INT32 ArchiveNameFindDrv(const TCHAR* pszSelArc);

// menu.cpp
#define UM_DISPLAYPOPUP (WM_USER + 0x0100)
#define UM_CANCELPOPUP  (WM_USER + 0x0101)

extern HANDLE hMenuThread;							// Handle to the thread that executes TrackPopupMenuEx
extern DWORD nMenuThreadID;							// ID of the thread that executes TrackPopupMenuEx
extern HWND hMenubar;								// Handle to the Toolbar control comprising the menu
extern HWND hMenuWindow;
extern bool bMenuDisplayed;
extern int nLastMenu;
extern HMENU hMenu;									// Handle to the menu
extern HMENU hMenuPopup;							// Handle to a popup version of the menu
extern int nMenuHeight;
extern int bAutoPause;
extern int nScreenSize;
extern int nScreenSizeHor;	// For horizontal orientation
extern int nScreenSizeVer;	// For vertical orientation
extern int nWindowSize;

#define SHOW_PREV_GAMES		10
extern TCHAR szPrevGames[SHOW_PREV_GAMES][64];

extern bool bModelessMenu;
extern bool bAdaptivepopup;

int MenuCreate();
void MenuDestroy();
int SetMenuPriority();
void MenuUpdate();
void MenuUpdateVolume();
void MenuUpdateSlowMo();

void CreateArcaderesItem();
void MenuEnableItems();
bool MenuHandleKeyboard(MSG*);
void MenuRemoveTheme();

// sel.cpp
extern UINT64 nLoadMenuShowX;
extern int nLoadMenuShowY;
extern int nLoadMenuExpand;
extern int nLoadMenuBoardTypeFilter;
extern int nLoadMenuGenreFilter;
extern int nLoadMenuFavoritesFilter;
extern int nLoadMenuFamilyFilter;
extern int nSelDlgWidth;
extern int nSelDlgHeight;
int SelDialog(int nMVSCartsOnly, HWND hParentWND);
extern UINT_PTR nTimer;
extern HBITMAP hPrevBmp;
extern HBITMAP hTitleBmp;
extern int nDialogSelect;
extern int nOldDlgSelected;
void CreateToolTipForRect(HWND hwndParent, PTSTR pszText);
int SelMVSDialog();
#define		ICON_16x16			0
#define		ICON_24x24			1
#define		ICON_32x32			2
extern bool bEnableIcons;
extern bool bIconsLoaded;
extern bool bIconsOnlyParents;
extern int nIconsSize, nIconsSizeXY, nIconsYDiff;
extern bool bGameInfoOpen;
extern bool bIconsByHardwares;

extern HICON* pIconsCache;

void CreateDrvIconsCache();
void DestroyDrvIconsCache();

void LoadDrvIcons();
void UnloadDrvIcons();

// neocdsel.cpp
extern int NeoCDList_Init();
extern bool bNeoCDListScanSub;
extern bool bNeoCDListScanOnlyISO;
extern TCHAR szNeoCDCoverDir[MAX_PATH];
extern TCHAR szNeoCDGamesDir[MAX_PATH];

HBITMAP ImageToBitmap(HWND hwnd, IMAGE* img);
HBITMAP PNGLoadBitmap(HWND hWnd, FILE* fp, int nWidth, int nHeight, int nPreset);
HBITMAP LoadBitmap(HWND hWnd, FILE* fp, int nWidth, int nHeight, int nPreset);
int NeoCDList_CheckISO(TCHAR* pszFile, void (*pfEntryCallBack)(INT32, TCHAR*));

// romdata.cpp
extern bool bRDListScanSub;
TCHAR* _strqtoken(TCHAR* s, const TCHAR* delims);
INT32 RomdataGetDrvIndex(const TCHAR* pszDrvName);
TCHAR* RomdataGetZipName(const TCHAR* pszFileName);
TCHAR* RomdataGetDrvName(const TCHAR* pszFileName);
TCHAR* RomdataGetFullName(const TCHAR* pszFileName);
bool FindZipNameFromDats(const TCHAR* dirPath, const char* pszZipName, TCHAR* pszFindDat);
INT32 RomDataManagerInit();
bool RomDataSetQuickPath(const TCHAR* pszSelDat);
INT32 RomDataCheck(const TCHAR* pszDatFile);
void RomDataStateBackup();
void RomDataStateRestore();
bool RomDataExportTemplate(HWND hWnd, const INT32 nDrvSelect);

// cona.cpp
struct SubDirInfo {
	TCHAR   BaseDir[MAX_PATH];
	TCHAR** SubDirs;
	UINT32  nCount;
};

extern SubDirInfo _SubDirInfo[DIRS_MAX];

extern int nIniVersion;

struct VidPresetData { int nWidth; int nHeight; };
extern struct VidPresetData VidPreset[4];

struct VidPresetDataVer { int nWidth; int nHeight; };
extern struct VidPresetDataVer VidPresetVer[4];

INT32 LookupSubDirThreads();
void SubDirThreadExit();
void DestroySubDir();
int ConfigAppLoad();
int ConfigAppSave();

// wave.cpp
extern FILE* WaveLog;								// wave log file

int WaveLogStart();
int WaveLogStop();

// inpd.cpp
extern HWND hInpdDlg;								// Handle to the Input Dialog

int InpdUpdate();
int InpdCreate();
int InpdListMake(int bBuild);

// inpcheat.cpp
extern HWND hInpCheatDlg;							// Handle to the Input Dialog

int InpCheatCreate();
int InpCheatListMake(int bBuild);

// inpdipsw.cpp
extern HWND hInpDIPSWDlg;							// Handle to the Input Dialog
void InpDIPSWResetDIPs();
int InpDIPSWCreate();

// inpmacro.cpp
extern HWND hInpMacroDlg;
void InpMacroExit();
int InpMacroCreate(int nInput);

// inps.cpp
extern HWND hInpsDlg;								// Handle to the Input Set Dialog
extern unsigned int nInpsInput;						// The input number we are redefining
int InpsCreate();
int InpsUpdate();

// inpc.cpp
extern HWND hInpcDlg;								// Handle to the Input Constant Dialog
extern unsigned int nInpcInput;						// The input number we are redefining
int InpcCreate();

// stated.cpp
extern int bDrvSaveAll;
int StatedAuto(int bSave);
int StatedLoad(int nSlot);
int StatedUNDO(int nSlot);
int StatedSave(int nSlot);

// numdial.cpp
int NumDialCreate(int bDial);
void GammaDialog();
void HardFXShaderSettingsDialog();
void ScanlineDialog();
void PhosphorDialog();
void ScreenAngleDialog();
void CPUClockDialog();
void CubicSharpnessDialog();
// sfactd.cpp
int SFactdCreate();

// roms.cpp
extern INT32 nRomsDlgWidth;
extern INT32 nRomsDlgHeight;
extern char* gameAv;
extern bool avOk;
extern bool bSkipStartupCheck;
INT32 RomsDirCreate(HWND hParentWND);
INT32 CreateROMInfo(HWND hParentWND);
void FreeROMInfo();
INT32 WriteGameAvb();

// support_paths.cpp
extern INT32 nSupportDlgWidth;
extern INT32 nSupportDlgHeight;
INT32 SupportDirCreate(HWND hParentWND);
//int SupportDirCreateTab(int nTab, HWND hParentWND);

// res.cpp
int ResCreate(int);

// fba_kaillera.cpp
int KailleraInitInput();
int KailleraGetInput();

// replay.cpp
extern int nReplayStatus;
extern bool bReplayReadOnly;
extern bool bReplayFrameCounterDisplay;
extern INT32 movieFlags;
int RecordInput();
int ReplayInput();
int StartRecord();
int StartReplay(const TCHAR* szFileName = NULL);
void StopReplay();
INT32 FreezeInputSize();
int FreezeInput(unsigned char** buf, int* size);
int UnfreezeInput(const unsigned char* buf, int size);
void DisplayReplayProperties(HWND hDlg, bool bClear);

// memcard.cpp
extern int nMemoryCardStatus;						// & 1 = file selected, & 2 = inserted

int	MemCardCreate();
int	MemCardSelect();
int	MemCardInsert();
int	MemCardEject();
int	MemCardToggle();

// progress.cpp
int ProgressUpdateBurner(double dProgress, const TCHAR* pszText, bool bAbs);
int ProgressCreate();
int ProgressDestroy();

// gameinfo.cpp
int GameInfoDialogCreate(HWND hParentWND, int nDrvSel);
void LoadFavorites();
void AddFavorite_Ext(UINT8 addf);
INT32 CheckFavorites(char *name);

// luaconsole.cpp
extern HWND LuaConsoleHWnd;
void UpdateLuaConsole(const wchar_t* fname);

// ---------------------------------------------------------------------------
// Debugger

// debugger.cpp
extern HWND hDbgDlg;

int DebugExit();
int DebugCreate();

// paletteviewer.cpp
int PaletteViewerDialogCreate(HWND hParentWND);

// ips_manager.cpp
extern INT32 nIpsSelectedLanguage;
INT32 GetIpsNumPatches();
INT32 LoadIpsActivePatches();
INT32 IpsManagerCreate(HWND hParentWND);
INT32 IpsGetDrvForQuickOpen(const TCHAR* pszSelDat);

// localise_download.cpp
int LocaliseDownloadCreate(HWND hParentWND);

// choose_monitor.cpp
int ChooseMonitorCreate();

// placeholder.cpp
int SelectPlaceHolder();
void ResetPlaceHolder();

// Misc
#define _TtoA(a)	TCHARToANSI(a, NULL, 0)
#define _AtoT(a)	ANSIToTCHAR(a, NULL, 0)

#ifdef INCLUDE_AVI_RECORDING
// ----------------------------------------------------------------------------
// AVI recording

// avi.cpp
INT32 AviStart();
INT32 AviRecordFrame(INT32 bDraw);
void AviStop();
extern INT32 nAviStatus;
extern INT32 nAvi3x;
#endif
