//***************************************************************************************
// Collision.h by X_Jun(MKXJun) (C) 2018-2019 All Rights Reserved.
// Licensed under the MIT License.
//
// �ṩһЩ��װ�õĶ������ײ��ⷽ��
// ע�⣺WireFrameDataĿǰ��δ�����ȶ����ԣ�δ���п��ܻ���ֲ��Geometry.h��
// Provide encapsulated collision classes and detection method.
//***************************************************************************************

#ifndef COLLISION_H
#define COLLISION_H

#include <DirectXCollision.h>
#include <vector>
#include "Vertex.h"
#include "Camera.h"


struct Ray
{
	Ray();
	Ray(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction);

	static Ray ScreenToRay(const Camera& camera, float screenX, float screenY);

	bool Hit(const DirectX::BoundingBox& box, float* pOutDist = nullptr, float maxDist = FLT_MAX);
	bool Hit(const DirectX::BoundingOrientedBox& box, float* pOutDist = nullptr, float maxDist = FLT_MAX);
	bool Hit(const DirectX::BoundingSphere& sphere, float* pOutDist = nullptr, float maxDist = FLT_MAX);
	bool XM_CALLCONV Hit(DirectX::FXMVECTOR V0, DirectX::FXMVECTOR V1, DirectX::FXMVECTOR V2, float* pOutDist = nullptr, float maxDist = FLT_MAX);

	DirectX::XMFLOAT3 origin;		// ����ԭ��
	DirectX::XMFLOAT3 direction;	// ��λ��������
};


class Collision
{
public:

	// �߿򶥵�/��������
	struct WireFrameData
	{
		std::vector<VertexPosColor> vertexVec;		// ��������
		std::vector<WORD> indexVec;					// ��������
	};

	//
	// ��Χ���߿�Ĵ���
	//

	// ����AABB���߿�
	static WireFrameData CreateBoundingBox(const DirectX::BoundingBox& box, const DirectX::XMFLOAT4& color);
	// ����OBB���߿�
	static WireFrameData CreateBoundingOrientedBox(const DirectX::BoundingOrientedBox& box, const DirectX::XMFLOAT4& color);
	// ������Χ���߿�
	static WireFrameData CreateBoundingSphere(const DirectX::BoundingSphere& sphere, const DirectX::XMFLOAT4& color, int slices = 20);
	// ������׶���߿�
	static WireFrameData CreateBoundingFrustum(const DirectX::BoundingFrustum& frustum, const DirectX::XMFLOAT4& color);

	//
	// ���ֵȼ۵Ĳ�����׶��ü��ķ�������ȡ��������׶����ײ����ײ���Ӧ�������������
	//

	// ��׶��ü�
	static std::vector<DirectX::XMMATRIX> XM_CALLCONV FrustumCulling(
		const std::vector<DirectX::XMMATRIX>& Matrices, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj);
	// ��׶��ü�2
	static std::vector<DirectX::XMMATRIX> XM_CALLCONV FrustumCulling2(
		const std::vector<DirectX::XMMATRIX>& Matrices, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj);
	// ��׶��ü�3
	static std::vector<DirectX::XMMATRIX> XM_CALLCONV FrustumCulling3(
		const std::vector<DirectX::XMMATRIX>& Matrices, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj);

private:
	static WireFrameData CreateFromCorners(const DirectX::XMFLOAT3(&corners)[8], const DirectX::XMFLOAT4& color);
};





#endif
