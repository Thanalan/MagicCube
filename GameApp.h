#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Rubik.h"
#include "Camera.h"
#include <ctime>
#include <sstream>

class GameApp : public D3DApp
{
public:
	enum class GameStatus {
		Preparing,	// ׼����
		Ready,		// ����
		Playing,	// ������
		Finished,	// �����
	};

public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


private:

	// ���ڲ�������ħ��������
	void Shuffle();
	// �����������������ɶ���������true
	bool PlayCameraAnimation(float dt);

	bool InitResource();

	void KeyInput();
	void MouseInput(float dt);

	std::wstring floating_to_wstring(float val, int precision);

private:
	ComPtr<ID2D1SolidColorBrush> mColorBrush;	// ��ɫ��ˢ
	ComPtr<IDWriteFont> mFont;					// ����
	ComPtr<IDWriteTextFormat> mTextFormat;		// �ı���ʽ

	Rubik mRubik;								// ħ��
	
	std::unique_ptr<Camera> mCamera;			// �����˳������

	BasicEffect mBasicEffect;					// ������Ч������

	GameTimer mGameTimer;						// ��Ϸ��ʱ��
	GameStatus mGameStatus;						// ��Ϸ״̬
	bool mIsCompleted;							// �Ƿ����

	float mAnimationTime;						// ��������ʱ��

	//
	// ����������
	//
	
	int mClickPosX, mClickPosY;					// ���ε��ʱ���λ��
	float mSlideDelay;							// �϶��ӳ���Ӧʱ�� 
	float mCurrDelay;							// ��ǰ�ӳ�ʱ��
	bool mDirectionLocked;						// ������

	RubikRotationRecord mCurrRotationRecord;	// ��ǰ��ת��¼

	std::stack<RubikRotationRecord> mRotationRecordStack;	// ��ת��¼ջ
};


#endif