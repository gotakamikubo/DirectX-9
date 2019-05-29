
#include<Windows.h>
#include<d3dx9.h>
#include <tchar.h>
#include<dinput.h>


#define SAFE_RELEASE(p){if(p){(p)->Release();(p)=NULL;}}

LPDIRECT3D9 pD3d;
LPDIRECT3DDEVICE9 pDevice;
LPD3DXSPRITE pSprite;
LPDIRECT3DTEXTURE9 pTexture;
LPDIRECTINPUT8 pDinput;
LPDIRECTINPUTDEVICE8 pKeyDevice;
LPD3DXFONT m_pFont;
FLOAT fPosX = 270, fPosY = 180;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3d(HWND);
HRESULT InitDinput(HWND);
static const int MAX_KEY_NUMBER = 256;
const int MASK_NUM = 0x80;
BYTE KeyState[MAX_KEY_NUMBER];

VOID FreeDx();
VOID SetState();
void UpdateKeyStatus();
bool GetKeyStatus(int);


struct CUSTOMVERTEX {
	float	x, y, z;
	float	rhw;
	DWORD	dwColor;
	float	tu, tv;
};

#define SetFVFC  (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmdShow) {
	HWND hWnd = NULL;

	DWORD SyncPrev = timeGetTime();
	DWORD SyncCurr;

	MSG msg;

	CUSTOMVERTEX v[4] = {
	{10,10,0,1,0xFFFFFF,0,0},
	{200,10,0,1,0xFFFFFF,1,0},
	{200,200,0,1,0xFFFFFF,1,1},
	{10,200, 0,1,0xFFFFFF,0,1}
	};

	void InitPresentParameters(HWND);

	static char szAppName[] = "STEP3";

	WNDCLASS wndclass;

	//Window初期化情報の設定
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	//Windowの登録
	RegisterClass(&wndclass);
	//Windowの生成
	hWnd = CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, NULL, NULL, hInst, NULL);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	if (FAILED(InitD3d(hWnd))) {
		return 0;
	}

	if (FAILED(InitDinput(hWnd))) {
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
				SetState();
				//テクスチャ貼り付け開始
				pDevice->BeginScene();
				UpdateKeyStatus();
				//終了時の処理

				if (GetKeyStatus(DIK_RETURN)) {

					break;

				}

				//それぞれのキーに対応する方向に絵を動かす処理

				//上キーを押されたとき

				if (GetKeyStatus(DIK_UP)) {

					v[0].y -= 5.0f;

					v[1].y -= 5.0f;

					v[2].y -= 5.0f;

					v[3].y -= 5.0f;

				}

				//下キーを押されたとき

				if (GetKeyStatus(DIK_DOWN)) {

					v[0].y += 5.0f;

					v[1].y += 5.0f;

					v[2].y += 5.0f;

					v[3].y += 5.0f;

				}

				//左キーを押されたとき

				if (GetKeyStatus(DIK_LEFT)) {

					v[0].x -= 5.0f;

					v[1].x -= 5.0f;

					v[2].x -= 5.0f;

					v[3].x -= 5.0f;

				}

				//右キーを押されたとき

				if (GetKeyStatus(DIK_RIGHT)) {
					v[0].x += 5.0f;
					v[1].x += 5.0f;
					v[2].x += 5.0f;
					v[3].x += 5.0f;
				}

				pDevice->SetFVF(SetFVFC);

				pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, v, sizeof(CUSTOMVERTEX));
				//テクスチャの貼り付け
				pDevice->SetTexture(0, pTexture);
				//テクスチャの貼り付け完了
				pDevice->EndScene();
				//ウィンドウに表示
				pDevice->Present(NULL, NULL, NULL, NULL);

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

	d3dpp.BackBufferWidth = 0; //バックバッファの横幅
	d3dpp.BackBufferHeight = 0; // バックバッファの高さ
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; //バックバッファのフォーマット
	d3dpp.BackBufferCount = 1; //バックバッファの数
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE; //マルチサンプルのタイプ
	d3dpp.MultiSampleQuality = 0; //マルチサンプルのクオリティ
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //バッファーの入れ替え方法(バックバッファとフロントバッファ)
	d3dpp.hDeviceWindow = hWnd; //表示するウィンドウのハンドル
	d3dpp.Windowed = TRUE; //ウィンドウモード
	d3dpp.EnableAutoDepthStencil = FALSE; //深度バッファを管理するかどうかの設定
	d3dpp.Flags = 0; //バックバッファをロックするかどうか
	d3dpp.FullScreen_RefreshRateInHz = 0; //ディスプレイのリフレッシュレート
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &pDevice))) {
		MessageBox(0, "HALモードでDIRECT3Dデバイスを作成できません\nREFモードで再試行します", NULL, MB_OK);
		if (FAILED(pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_MIXED_VERTEXPROCESSING, &d3dpp, &pDevice))) {
			MessageBox(0, "DIRECT3Dデバイスの作成に失敗しました", NULL, MB_OK);
			return E_FAIL;
		}
	}

	if (FAILED(D3DXCreateTextureFromFile(pDevice, "7153.png", &pTexture))) {
		MessageBox(0, "テクスチャの作成に失敗しました", "", MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT InitDinput(HWND hWnd) {
	HRESULT hr;
	if(FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID * *)& pDinput, NULL))) {
		return hr;
	}
	if (FAILED(hr = pDinput->CreateDevice(GUID_SysKeyboard,&pKeyDevice, NULL))) {
		return hr;
	}

	if (FAILED(hr = pKeyDevice->SetDataFormat(&c_dfDIKeyboard))) {
		return hr;
	}

	if (FAILED(hr = pKeyDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND))) {
		return hr;
	}

	pKeyDevice->Acquire();
	return S_OK;
	
}


VOID FreeDx() {
	if (pKeyDevice)
	{
		pKeyDevice->Unacquire();
	}
	pKeyDevice->Release();
	pKeyDevice = nullptr;
	pDinput->Release();
	pDinput = nullptr;
	pDevice->Release();
	pDevice = nullptr;
	pD3d->Release();
	pD3d = nullptr;
	pTexture->Release();
	pTexture = nullptr;
}
VOID SetState() {

	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void UpdateKeyStatus() {
	HRESULT hr = pKeyDevice->Acquire();

	if ((hr == DI_OK) || (hr == S_FALSE)) {

		
		pKeyDevice->GetDeviceState(sizeof(KeyState), &KeyState);

	}

}


bool GetKeyStatus(int KeyNumber) {
	if (KeyState[KeyNumber] & 0x80) {
		return true;
	}
	return false;
}

