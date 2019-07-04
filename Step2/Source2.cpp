#include<Windows.h>
#include<d3dx9.h>

#pragma comment(lib,"winmm.lib")

#define SAFE_RELEASE(p){if(p){(p)->Release();(p)=NULL;}}

LPDIRECT3D9 pD3d;
LPDIRECT3D9 pDirect3D;
LPDIRECT3DDEVICE9 pDevice;
LPDIRECT3DTEXTURE9 pTexture;
LPD3DXSPRITE pSprite;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3d(HWND);
VOID FreeDx();

struct CUSTOMVERTEX {
	float	x, y, z;
	float	rhw;
	DWORD	dwColor;
	float	tu, tv;
};

CUSTOMVERTEX v[3] = {
	{10,10,0,1,0xFFFFFFFF,0,0},
	{200,10,0,1,0xFFFFFFFF,1,0},
	{10,200,0,1,0xFFFFFFFF,1,1}
};

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmdShow) {
	HWND hWnd = NULL;


#define SetFVFC (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

	DWORD SyncPrev = timeGetTime();
	DWORD SyncCurr;

	MSG msg;
	void InitPresentParameters(HWND);
	static char szAppName[] = "STEP2";
	WNDCLASSEX wndclass;
	//Window初期化情報の設定
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	//Windowの登録
	RegisterClassEx(&wndclass);
	//Windowの生成
	hWnd = CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, hInst, NULL);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);


	if (FAILED(InitD3d(hWnd))) {
		return 0;
	}
	timeBeginPeriod(1);
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			SyncCurr = timeGetTime();
			//1秒間に60回この中に入る
			if (SyncCurr - SyncPrev >= 1000 / 60) {
				//ウィンドウを黒色でクリア
				pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0, 0);

				pDevice->SetFVF(SetFVFC);
				//テクスチャ貼り付け開始
				pDevice->BeginScene();
				//テクスチャの貼り付け
				pDevice->SetTexture(0, pTexture);

				pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(CUSTOMVERTEX));
				//テクスチャの貼り付け完了
				pDevice->EndScene();
				//ウィンドウに表示
				pDevice->Present(0, 0, 0, 0);

				SyncPrev = SyncCurr;
			}
		}
	}
	timeEndPeriod(1);
	FreeDx();
	return (INT)msg.wParam;
}
//コールバック関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch ((CHAR)wParam)
		{
			//ESCを押して実行終了
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}


HRESULT InitD3d(HWND hWnd) {
	if (NULL == (pD3d = Direct3DCreate9(D3D_SDK_VERSION))) {
		MessageBox(0, "Direct3Dの作成に失敗しました", "", MB_OK);
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.Windowed = TRUE;

	if (FAILED(pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &pDevice))) {
		MessageBox(0, "HALモードでDIRECT3Dデバイスを作成できません\nREFモードで再試行します", NULL, MB_OK);
		if (FAILED(pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &pDevice))) {
			MessageBox(0, "DIRECT3Dデバイスの作成に失敗しました", NULL, MB_OK);
			return E_FAIL;
		}

		if (FAILED(D3DXCreateTextureFromFileEx(pDevice, NULL, 100, 100, 0, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_DEFAULT, NULL, NULL, NULL, &pTexture))) {
			MessageBox(0, "テクスチャの作成に失敗しました", "", MB_OK);
			return E_FAIL;
		}

	}return S_OK;
}

VOID FreeDx() {

	pDevice->Release();

	pDevice = nullptr;

	pD3d->Release();

	pD3d = nullptr;
}