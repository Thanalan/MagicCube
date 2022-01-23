//***************************************************************************************
// Camera.h by X_Jun(MKXJun) (C) 2018-2019 All Rights Reserved.
// Licensed under the MIT License.
//
// �ṩ��һ�˳�(�����ӽ�)�͵����˳������
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include <d3d11_1.h>
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	virtual ~Camera() = 0;

	// ��ȡ�����λ��
	DirectX::XMVECTOR GetPositionXM() const;
	DirectX::XMFLOAT3 GetPosition() const;

	// ��ȡ�����������������
	DirectX::XMVECTOR GetRightXM() const;
	DirectX::XMFLOAT3 GetRight() const;
	DirectX::XMVECTOR GetUpXM() const;
	DirectX::XMFLOAT3 GetUp() const;
	DirectX::XMVECTOR GetLookXM() const;
	DirectX::XMFLOAT3 GetLook() const;

	// ��ȡ��׶����Ϣ
	float GetNearWindowWidth() const;
	float GetNearWindowHeight() const;
	float GetFarWindowWidth() const;
	float GetFarWindowHeight() const;

	// ��ȡ����
	DirectX::XMMATRIX GetViewXM() const;
	DirectX::XMMATRIX GetProjXM() const;
	DirectX::XMMATRIX GetViewProjXM() const;

	// ��ȡ�ӿ�
	D3D11_VIEWPORT GetViewPort() const;


	// ������׶��
	void SetFrustum(float fovY, float aspect, float nearZ, float farZ);

	// �����ӿ�
	void SetViewPort(const D3D11_VIEWPORT& viewPort);
	void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

	// ���¹۲����
	virtual void UpdateViewMatrix() = 0;
protected:
	// ������Ĺ۲�ռ�����ϵ��Ӧ����������ϵ�еı�ʾ
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mRight;
	DirectX::XMFLOAT3 mUp;
	DirectX::XMFLOAT3 mLook;
	
	// ��׶������
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;

	// �۲�����͸��ͶӰ����
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	// ��ǰ�ӿ�
	D3D11_VIEWPORT mViewPort;

};

class FirstPersonCamera : public Camera
{
public:
	FirstPersonCamera();
	~FirstPersonCamera() override;

	// ���������λ��
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);
	// ����������ĳ���
	void XM_CALLCONV LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target,const DirectX::XMFLOAT3& up);
	void XM_CALLCONV LookTo(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR to, DirectX::FXMVECTOR up);
	void LookTo(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);
	// ƽ��
	void Strafe(float d);
	// ֱ��(ƽ���ƶ�)
	void Walk(float d);
	// ǰ��(��ǰ���ƶ�)
	void MoveForward(float d);
	// ���¹۲�
	void Pitch(float rad);
	// ���ҹ۲�
	void RotateY(float rad);


	// ���¹۲����
	void UpdateViewMatrix() override;
};

class ThirdPersonCamera : public Camera
{
public:
	ThirdPersonCamera();
	~ThirdPersonCamera() override;

	// ��ȡ��ǰ���������λ��
	DirectX::XMFLOAT3 GetTargetPosition() const;
	// ��ȡ������ľ���
	float GetDistance() const;
	// ��ȡ��X�����ת����
	float GetRotationX() const;
	// ��ȡ��Y�����ת����
	float GetRotationY() const;
	// �����崹ֱ��ת(ע��������Ұ�Ƕ�Phi������[pi/6, pi/2])
	void RotateX(float rad);
	// ������ˮƽ��ת
	void RotateY(float rad);
	// ��������
	void Approach(float dist);
	// ���ó�ʼ��X��Ļ���(ע��������Ұ�Ƕ�Phi������[pi/6, pi/2])
	void SetRotationX(float phi);
	// ���ó�ʼ��Y��Ļ���
	void SetRotationY(float theta);
	// ���ò��󶨴����������λ��
	void SetTarget(const DirectX::XMFLOAT3& target);
	// ���ó�ʼ����
	void SetDistance(float dist);
	// ������С����������
	void SetDistanceMinMax(float minDist, float maxDist);
	// ���¹۲����
	void UpdateViewMatrix() override;

private:
	DirectX::XMFLOAT3 mTarget;
	float mDistance;
	// ��С������룬����������
	float mMinDist, mMaxDist;
	// ����������ϵΪ��׼����ǰ����ת�Ƕ�
	float mTheta;
	float mPhi;
};


#endif
