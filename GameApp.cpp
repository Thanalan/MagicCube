#include "GameApp.h"
#include "d3dUtil.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	if (!mBasicEffect.InitAll(md3dDevice))
		return false;

	if (!InitResource())
		return false;

	// ��ʼ����꣬���̲���Ҫ
	mMouse->SetWindow(mhMainWnd);
	mMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	// ��ʼ�������ӳ�ʱ��͵��λ��
	mSlideDelay = 0.05f;
	mClickPosX = mClickPosY = -1;
	// ��ʼ����ʱ��
	mGameTimer.Reset();
	mGameTimer.Stop();
	// ��ʼ����Ϸ״̬
	mGameStatus = GameStatus::Preparing;
	mCurrRotationRecord.dTheta = 0.0f;
	return true;
}

void GameApp::OnResize()
{
	assert(md2dFactory);
	assert(mdwriteFactory);
	// �ͷ�D2D�������Դ
	mColorBrush.Reset();
	md2dRenderTarget.Reset();

	D3DApp::OnResize();

	// ΪD2D����DXGI������ȾĿ��
	ComPtr<IDXGISurface> surface;
	HR(mSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HRESULT hr = md2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, md2dRenderTarget.GetAddressOf());
	surface.Reset();

	if (hr == E_NOINTERFACE)
	{
		OutputDebugString(L"\n���棺Direct2D��Direct3D�������Թ������ޣ��㽫�޷������ı���Ϣ�����ṩ������ѡ������\n"
			"1. ����Win7ϵͳ����Ҫ������Win7 SP1������װKB2670838������֧��Direct2D��ʾ��\n"
			"2. �������Direct3D 10.1��Direct2D�Ľ�����������ģ�"
			"https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/direct2d-and-direct3d-interoperation-overview""\n"
			"3. ʹ�ñ������⣬����FreeType��\n\n");
	}
	else if (hr == S_OK)
	{
		// �����̶���ɫˢ���ı���ʽ
		HR(md2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			mColorBrush.GetAddressOf()));
		HR(mdwriteFactory->CreateTextFormat(L"����", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"zh-cn",
			mTextFormat.GetAddressOf()));
	}
	else
	{
		// �����쳣����
		assert(md2dRenderTarget);
	}

	// ����������ʾ
	if (mCamera != nullptr)
	{
		mCamera->SetFrustum(XM_PI * 0.4f, AspectRatio(), 1.0f, 1000.0f);
		mCamera->SetViewPort(0.0f, 0.0f, (float)mClientWidth, (float)mClientHeight);
		mBasicEffect.SetProjMatrix(mCamera->GetProjXM());
	}
}

void GameApp::UpdateScene(float dt)
{
	// �������
	if (mGameStatus == GameStatus::Preparing)
	{
		// �������������
		bool animComplete = PlayCameraAnimation(dt);

		if (!mRubik.IsLocked())
		{
			if (!mRotationRecordStack.empty())
			{
				// ����
				auto record = mRotationRecordStack.top();
				switch (record.axis)
				{
				case RubikRotationAxis_X: mRubik.RotateX(record.pos, -record.dTheta); break;
				case RubikRotationAxis_Y: mRubik.RotateY(record.pos, -record.dTheta); break;
				case RubikRotationAxis_Z: mRubik.RotateZ(record.pos, -record.dTheta); break;
				}
				mRotationRecordStack.pop();
			}
			else if (animComplete)
			{
				mGameStatus = GameStatus::Ready;
			}
		}
	}
	else
	{
		KeyInput();
		MouseInput(dt);
	}

	// ��ʵ������ת�Ż��ʱ
	if (mGameStatus == GameStatus::Ready && !mRotationRecordStack.empty())
	{
		// ��ʼ��Ϸ����ʱ
		mGameTimer.Start();
		mGameStatus = GameStatus::Playing;
	}
	else if (mGameStatus == GameStatus::Playing)
	{
		if (mRubik.IsCompleted() && !mRubik.IsLocked())
		{
			// ���ħ����ֹͣ��ʱ
			mGameTimer.Stop();
			mGameStatus = GameStatus::Finished;
			mIsCompleted = true;
			std::wstring wstr = L"������ʱ��" + floating_to_wstring(mGameTimer.TotalTime(), 3) + L"�롣";
			MessageBox(nullptr, wstr.c_str(), L"���", MB_OK);
		}
		else
		{
			mGameTimer.Tick();
		}
	}

	// ����ħ��
	mRubik.Update(dt);

}

void GameApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	// ʹ��ƫ��ɫ�Ĵ�ɫ����
	float backgroundColor[4] = { 0.45882352f, 0.42745098f, 0.51372549f, 1.0f };
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), backgroundColor);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	// ����ħ��
	mRubik.Draw(md3dImmediateContext, mBasicEffect);


	//
	// ����Direct2D����
	//
	if (md2dRenderTarget != nullptr)
	{
		md2dRenderTarget->BeginDraw();

		// ����Debug���
		Mouse::State mouseState = mMouse->GetState();
		std::wstring wstr = L"F10(һ����ԭ) F11(������Ϸ) F12(��������)\n��ʱ��" 
			+ floating_to_wstring(mGameTimer.TotalTime(), 3) + L"s";
		md2dRenderTarget->DrawTextW(wstr.c_str(), (UINT)wstr.size(), mTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, mColorBrush.Get());
		HR(md2dRenderTarget->EndDraw());
	}

	HR(mSwapChain->Present(0, 0));
}



void GameApp::Shuffle()
{
	// ��ջ
	while (!mRotationRecordStack.empty())
		mRotationRecordStack.pop();
	// ��ջ����30�������ת�������ڴ���
	RubikRotationRecord record;
	srand(static_cast<unsigned>(time(nullptr)));
	for (int i = 0; i < 30; ++i)
	{
		record.axis = static_cast<RubikRotationAxis>(rand() % 3);
		record.pos = rand() % 4;
		record.dTheta = XM_PIDIV2 * (rand() % 2 ? 1 : -1);
		mRotationRecordStack.push(record);
	}
}

bool GameApp::PlayCameraAnimation(float dt)
{
	// ��ȡ����
	auto cam3rd = dynamic_cast<ThirdPersonCamera*>(mCamera.get());

	// ******************
	// �����˳�������Ĳ���
	//
	mAnimationTime += dt;
	float theta, dist;

	theta = -XM_PIDIV2 + XM_PIDIV4 * mAnimationTime * 0.2f;
	dist = 20.0f - mAnimationTime * 2.0f;
	if (theta > -XM_PIDIV4)
		theta = -XM_PIDIV4;
	if (dist < 10.0f)
		dist = 10.0f;

	cam3rd->SetRotationY(theta);
	cam3rd->SetDistance(dist);

	// ���¹۲����
	mCamera->UpdateViewMatrix();
	mBasicEffect.SetViewMatrix(mCamera->GetViewXM());

	if (fabs(theta + XM_PIDIV4) < 1e-5f && fabs(dist - 10.0f) < 1e-5f)
		return true;
	return false;
}


bool GameApp::InitResource()
{
	// �������ڴ���ħ���ļ�¼
	Shuffle();
	// ��ʼ��ħ��
	mRubik.InitResources(md3dDevice, md3dImmediateContext);
	mRubik.SetRotationSpeed(XM_2PI * 1.5f);
	// ��ʼ����Ч
	mBasicEffect.SetRenderDefault(md3dImmediateContext);
	// ��ʼ�������
	mCamera.reset(new ThirdPersonCamera);
	auto cam3rd = dynamic_cast<ThirdPersonCamera*>(mCamera.get());
	cam3rd->SetDistance(10.0f);
	cam3rd->SetDistanceMinMax(10.0f, 200.0f);
	cam3rd->SetFrustum(XM_PI * 0.4f, AspectRatio(), 1.0f, 1000.0f);
	cam3rd->SetViewPort(0.0f, 0.0f, (float)mClientWidth, (float)mClientHeight);
	cam3rd->SetTarget(XMFLOAT3(0.0f, 0.0f, 0.0f));
	cam3rd->SetRotationX(XM_PIDIV2 * 0.6f);
	mBasicEffect.SetProjMatrix(cam3rd->GetProjXM());
	mBasicEffect.SetTextureArray(mRubik.GetTexArray());
	

	return true;
}

void GameApp::KeyInput()
{
	Keyboard::State keyState = mKeyboard->GetState();
	mKeyboardTracker.Update(keyState);

	//
	// �������
	//

	// ������ԭ��������ɼ�
	if (mKeyboardTracker.IsKeyPressed(Keyboard::F10))
	{
		
		mGameTimer.Reset();
		mGameTimer.Stop();
		mRubik.Reset();
		mGameStatus = GameStatus::Finished;
	}
	// ������Ϸ
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::F11))
	{
		mGameTimer.Reset();
		mGameTimer.Stop();
		mRubik.Reset();
		mGameStatus = GameStatus::Preparing;
		Shuffle();
		mAnimationTime = 0.0f;
	}
	else if (mKeyboardTracker.IsKeyPressed(Keyboard::F12))
	{
		std::wstring wstr = L"���ߣ�    \n"
			"�汾��v1.0\n"
			"��ħ���ɹ�ѧϰ������\n";
		MessageBox(nullptr, wstr.c_str(), L"��������", MB_OK);
	}

	//
	// ��������
	//
	if (keyState.IsKeyDown(Keyboard::LeftControl) &&
		mKeyboardTracker.IsKeyPressed(Keyboard::Z) &&
		!mRubik.IsLocked() && !mRotationRecordStack.empty())
	{
		auto record = mRotationRecordStack.top();
		switch (record.axis)
		{
		case RubikRotationAxis_X: mRubik.RotateX(record.pos, -record.dTheta); break;
		case RubikRotationAxis_Y: mRubik.RotateY(record.pos, -record.dTheta); break;
		case RubikRotationAxis_Z: mRubik.RotateZ(record.pos, -record.dTheta); break;
		}
		mRotationRecordStack.pop();
	}

	//
	// ����ħ����ת
	//

	// ��ʱ������ת�Ļ�����ǰ����
	if (mRubik.IsLocked())
		return;

	// ��ʽx
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Up))
	{
		mRubik.RotateX(3, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 3, XM_PIDIV2 });
		return;
	}
	// ��ʽx'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Down))
	{
		mRubik.RotateX(3, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 3, -XM_PIDIV2 });
		return;
	}
	// ��ʽy
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Left))
	{
		mRubik.RotateY(3, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 3, XM_PIDIV2 });
		return;
	}
	// ��ʽy'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Right))
	{
		mRubik.RotateY(3, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 3, -XM_PIDIV2 });
		return;
	}
	// ��ʽz'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::PageUp))
	{
		mRubik.RotateZ(3, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 3, XM_PIDIV2 });
		return;
	}
	// ��ʽz
	if (mKeyboardTracker.IsKeyPressed(Keyboard::PageDown))
	{
		mRubik.RotateZ(3, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 3, -XM_PIDIV2 });
		return;
	}

	//
	// ˫����ת
	//

	// ��ʽr
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::I))
	{
		mRubik.RotateX(-2, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, -2, XM_PIDIV2 });
		return;
	}
	// ��ʽr'
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::K))
	{
		mRubik.RotateX(-2, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, -2, -XM_PIDIV2 });
		return;
	}
	// ��ʽu
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::J))
	{
		mRubik.RotateY(-2, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, -2, XM_PIDIV2 });
		return;
	}
	// ��ʽu'
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::L))
	{
		mRubik.RotateY(-2, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, -2, -XM_PIDIV2 });
		return;
	}
	// ��ʽf'
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::U))
	{
		mRubik.RotateZ(-1, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, -1, XM_PIDIV2 });
		return;
	}
	// ��ʽf
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::O))
	{
		mRubik.RotateZ(-1, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, -1, -XM_PIDIV2 });
		return;
	}

	// ��ʽl'
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::W))
	{
		mRubik.RotateX(-1, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, -1, XM_PIDIV2 });
		return;
	}
	// ��ʽl
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::S))
	{
		mRubik.RotateX(-1, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, -1, -XM_PIDIV2 });
		return;
	}
	// ��ʽd'
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::A))
	{
		mRubik.RotateY(-1, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, -1, XM_PIDIV2 });
		return;
	}
	// ��ʽd
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::D))
	{
		mRubik.RotateY(-1, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, -1, -XM_PIDIV2 });
		return;
	}
	// ��ʽb
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::Q))
	{
		mRubik.RotateZ(-2, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, -2, XM_PIDIV2 });
		return;
	}
	// ��ʽb'
	if (keyState.IsKeyDown(Keyboard::LeftControl) && mKeyboardTracker.IsKeyPressed(Keyboard::E))
	{
		mRubik.RotateZ(-2, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, -2, -XM_PIDIV2 });
		return;
	}


	//
	// ������ת
	//

	// ��ʽR
	if (mKeyboardTracker.IsKeyPressed(Keyboard::I))
	{
		mRubik.RotateX(2, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 2, XM_PIDIV2 });
		return;
	}
	// ��ʽR'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::K))
	{
		mRubik.RotateX(2, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 2, -XM_PIDIV2 });
		return;
	}
	// ��ʽU
	if (mKeyboardTracker.IsKeyPressed(Keyboard::J))
	{
		mRubik.RotateY(2, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 2, XM_PIDIV2 });
		return;
	}
	// ��ʽU'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::L))
	{
		mRubik.RotateY(2, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 2, -XM_PIDIV2 });
		return;
	}
	// ��ʽF'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::U))
	{
		mRubik.RotateZ(0, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 0, XM_PIDIV2 });
		return;
	}
	// ��ʽF
	if (mKeyboardTracker.IsKeyPressed(Keyboard::O))
	{
		mRubik.RotateZ(0, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 0, -XM_PIDIV2 });
		return;
	}

	// ��ʽL'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::W))
	{
		mRubik.RotateX(0, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 0, XM_PIDIV2 });
		return;
	}
	// ��ʽL
	if (mKeyboardTracker.IsKeyPressed(Keyboard::S))
	{
		mRubik.RotateX(0, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 0, -XM_PIDIV2 });
		return;
	}
	// ��ʽD'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::A))
	{
		mRubik.RotateY(0, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 0, XM_PIDIV2 });
		return;
	}
	// ��ʽD
	if (mKeyboardTracker.IsKeyPressed(Keyboard::D))
	{
		mRubik.RotateY(0, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 0, -XM_PIDIV2 });
		return;
	}
	// ��ʽB
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Q))
	{
		mRubik.RotateZ(2, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 2, XM_PIDIV2 });
		return;
	}
	// ��ʽB'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::E))
	{
		mRubik.RotateZ(2, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 2, -XM_PIDIV2 });
		return;
	}

	// ��ʽM
	if (mKeyboardTracker.IsKeyPressed(Keyboard::T))
	{
		mRubik.RotateX(1, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 1, XM_PIDIV2 });
		return;
	}
	// ��ʽM'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::G))
	{
		mRubik.RotateX(1, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_X, 1, -XM_PIDIV2 });
		return;
	}
	// ��ʽE
	if (mKeyboardTracker.IsKeyPressed(Keyboard::F))
	{
		mRubik.RotateY(1, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 1, XM_PIDIV2 });
		return;
	}
	// ��ʽE'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::H))
	{
		mRubik.RotateY(1, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Y, 1, -XM_PIDIV2 });
		return;
	}
	// ��ʽS'
	if (mKeyboardTracker.IsKeyPressed(Keyboard::R))
	{
		mRubik.RotateZ(1, XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 1, XM_PIDIV2 });
		return;
	}
	// ��ʽS
	if (mKeyboardTracker.IsKeyPressed(Keyboard::Y))
	{
		mRubik.RotateZ(1, -XM_PIDIV2);
		mRotationRecordStack.push(RubikRotationRecord{ RubikRotationAxis_Z, 1, -XM_PIDIV2 });
		return;
	}
	
}

void GameApp::MouseInput(float dt)
{
	Mouse::State mouseState = mMouse->GetState();
	Mouse::State lastState = mMouseTracker.GetLastState();
	mMouseTracker.Update(mouseState);
	

	int dx = mouseState.x - lastState.x;
	int dy = mouseState.y - lastState.y;
	// ��ȡ����
	auto cam3rd = dynamic_cast<ThirdPersonCamera*>(mCamera.get());

	// ******************
	// �����˳�������Ĳ���
	//

	// ��������ת�������΢����
	cam3rd->SetRotationX(XM_PIDIV2 * 0.6f + (mouseState.y - mClientHeight / 2) *  0.0001f);
	cam3rd->SetRotationY(-XM_PIDIV4 + (mouseState.x - mClientWidth / 2) * 0.0001f);
	cam3rd->Approach(-mouseState.scrollWheelValue / 120 * 1.0f);

	// ���¹۲����
	mCamera->UpdateViewMatrix();
	mBasicEffect.SetViewMatrix(mCamera->GetViewXM());

	// ���ù���ֵ
	mMouse->ResetScrollWheelValue();

	// ******************
	// ħ������
	//

	// �������Ƿ���
	if (mouseState.leftButton)
	{
		// ��ʱδȷ����ת����
		if (!mDirectionLocked)
		{
			// ��ʱδ��¼���λ��
			if (mClickPosX == -1 && mClickPosY == -1)
			{
				// ���ε��
				if (mMouseTracker.leftButton == Mouse::ButtonStateTracker::PRESSED)
				{
					// ��¼���λ��
					mClickPosX = mouseState.x;
					mClickPosY = mouseState.y;
				}
			}
			
			// ������¼�˵��λ�òŽ��и���
			if (mClickPosX != -1 && mClickPosY != -1)
				mCurrDelay += dt;
			// δ���ﻬ���ӳ�ʱ�������
			if (mCurrDelay < mSlideDelay)
				return;

			// δ�����˶�������
			if (abs(dx) == abs(dy))
				return;

			// ��ʼ�Ϸ�����
			mDirectionLocked = true;
			// �����ۻ���λ�Ʊ仯��
			dx = mouseState.x - mClickPosX;
			dy = mouseState.y - mClickPosY;

			// �ҵ���ǰ������ķ�������
			Ray ray = Ray::ScreenToRay(*mCamera, (float)mouseState.x, (float)mouseState.y);
			float dist;
			XMINT3 pos = mRubik.HitCube(ray, &dist);

			// �жϵ�ǰ��Ҫ�Ǵ�ֱ��������ˮƽ����
			bool isVertical = abs(dx) < abs(dy);
			// ��ǰ�����ݵ���-Z�棬���ݲ������;�����ת��
			if (pos.z == 0 && fabs((ray.origin.z + dist * ray.direction.z) - (-3.0f)) < 1e-5f)
			{
				mCurrRotationRecord.pos = isVertical ? pos.x : pos.y;
				mCurrRotationRecord.axis = isVertical ? RubikRotationAxis_X : RubikRotationAxis_Y;
			}
			// ��ǰ�����ݵ���+X�棬���ݲ������;�����ת��
			else if (pos.x == 2 && fabs((ray.origin.x + dist * ray.direction.x) - 3.0f) < 1e-5f)
			{
				mCurrRotationRecord.pos = isVertical ? pos.z : pos.y;
				mCurrRotationRecord.axis = isVertical ? RubikRotationAxis_Z : RubikRotationAxis_Y;
			}
			// ��ǰ�����ݵ���+Y�棬Ҫ�ж�ƽ�Ʊ仯��dx��dy�ķ�����������ת����
			else if (pos.y == 2 && fabs((ray.origin.y + dist * ray.direction.y) - 3.0f) < 1e-5f)
			{
				// �ж����
				bool diffSign = ((dx & 0x80000000) != (dy & 0x80000000));
				mCurrRotationRecord.pos = diffSign ? pos.x : pos.z;
				mCurrRotationRecord.axis = diffSign ? RubikRotationAxis_X : RubikRotationAxis_Z;
			}
			// ��ǰ�����ݵ��ǿհ׵������������ħ����ת
			else
			{
				mCurrRotationRecord.pos = 3;
				// ˮƽ������Y����ת
				if (!isVertical)
				{
					mCurrRotationRecord.axis = RubikRotationAxis_Y;
				}
				// ��Ļ��벿�ֵĴ�ֱ������X����ת
				else if (mouseState.x < mClientWidth / 2)
				{
					mCurrRotationRecord.axis = RubikRotationAxis_X;
				}
				// ��Ļ�Ұ벿�ֵĴ�ֱ������Z����ת
				else
				{
					mCurrRotationRecord.axis = RubikRotationAxis_Z;
				}
			}
		}

		// ���˷��������ܽ�����ת
		if (mDirectionLocked)
		{
			// ������ת
			switch (mCurrRotationRecord.axis)
			{
			case RubikRotationAxis_X: mRubik.RotateX(mCurrRotationRecord.pos, (dx - dy) * 0.008f, true);
				mCurrRotationRecord.dTheta += (dx - dy) * 0.008f;
				break;
			case RubikRotationAxis_Y: mRubik.RotateY(mCurrRotationRecord.pos, -dx * 0.008f, true);
				mCurrRotationRecord.dTheta += (-dx * 0.008f);
				break;
			case RubikRotationAxis_Z: mRubik.RotateZ(mCurrRotationRecord.pos, (-dx - dy) * 0.008f, true);
				mCurrRotationRecord.dTheta += (-dx - dy) * 0.008f;
				break;
			}
		}
	}
	// �����ͷ�
	else if (mMouseTracker.leftButton == Mouse::ButtonStateTracker::RELEASED)
	{
		// �ͷŷ�����
		mDirectionLocked = false;
		// �����ӳٹ���
		mCurrDelay = 0.0f;
		// �����Ƴ���Ļ
		mClickPosX = mClickPosY = -1;

		// �������ָ�����Ԥ��ת
		switch (mCurrRotationRecord.axis)
		{
		case RubikRotationAxis_X: mRubik.RotateX(mCurrRotationRecord.pos, 0.0f); break;
		case RubikRotationAxis_Y: mRubik.RotateY(mCurrRotationRecord.pos, 0.0f); break;
		case RubikRotationAxis_Z: mRubik.RotateZ(mCurrRotationRecord.pos, 0.0f); break;
		}

		// �������ת�����壬��¼��ջ��
		int times = static_cast<int>(round(mCurrRotationRecord.dTheta / XM_PIDIV2)) % 4;
		if (times != 0)
		{
			mCurrRotationRecord.dTheta = times * XM_PIDIV2;
			mRotationRecordStack.push(mCurrRotationRecord);
		}
		// ��תֵ����
		mCurrRotationRecord.dTheta = 0.0f;
	}
}

std::wstring GameApp::floating_to_wstring(float val, int precision)
{
	std::wostringstream oss;
	oss.setf(std::ios::fixed);
	oss.precision(precision);
	oss << val;
	return oss.str();
}

