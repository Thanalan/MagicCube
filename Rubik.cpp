#include "Rubik.h"
#include "d3dUtil.h"
#include "Vertex.h"
#include <fstream>
using namespace DirectX;
using namespace Microsoft::WRL;

DirectX::XMMATRIX Cube::GetWorldMatrix() const
{
	XMVECTOR posVec = XMLoadFloat3(&pos);
	// rotation��Ȼ���ֻ��һ�������Ƿ�0����֤��ֻ��������һ���������ת
	XMMATRIX R = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	posVec = XMVector3TransformCoord(posVec, R);
	// ������ת�������յ�λ��
	XMFLOAT3 finalPos;
	XMStoreFloat3(&finalPos, posVec);

	return XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		XMMatrixTranslation(finalPos.x, finalPos.y, finalPos.z);
}

Rubik::Rubik()
	: mRotationSpeed(XM_2PI)
{
}

void Rubik::InitResources(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> deviceContext)
{

	std::vector<std::wstring> filenames {
		L"Resource/Black.dds",
		L"Resource/Orange.dds",
		L"Resource/Red.dds",
		L"Resource/Green.dds",
		L"Resource/Blue.dds",
		L"Resource/Yellow.dds",
		L"Resource/White.dds",
	};

	// ���������ļ��Ƿ����
	bool fileExists = true;
	for (const std::wstring& filename : filenames)
	{
		std::wifstream wfin(filename);
		if (!wfin.is_open())
		{
			fileExists = false;
			wfin.close();
			break;
		}
		wfin.close();
	}
	if (fileExists)
	{
		// ���ļ���ȡ
		mTexArray = CreateDDSTexture2DArrayFromFile(device, deviceContext, filenames);
	}
	else
	{
		// ���ڴ��ȡ
		// �������ܻ�дר�ŵ�ͨ�ú���
		mTexArray = CreateRubikCubeTextureArrayFromMemory(device, deviceContext);
	}

	//
	// ��ʼ������������ģ��
	//

	VertexPosTex vertices[] = {
		// +X��
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		// -X��
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		// +Y��
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		// -Y��
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		// +Z��
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		// -Z��
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	// ���ö��㻺��������
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof vertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// �½����㻺����
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	HR(device->CreateBuffer(&vbd, &initData, mVertexBuffer.ReleaseAndGetAddressOf()));
	

	WORD indices[] = { 0, 1, 2, 2, 3, 0 };
	// ������������������
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof indices;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// �½�����������
	initData.pSysMem = indices;
	HR(device->CreateBuffer(&ibd, &initData, mIndexBuffer.ReleaseAndGetAddressOf()));

	// ��ʼ��ħ��������
	Reset();

	// Ԥ�Ȱ󶨶���/��������������Ⱦ����
	UINT strides[1] = { sizeof(VertexPosTex) };
	UINT offsets[1] = { 0 };
	deviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), strides, offsets);
	deviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

}

void Rubik::Reset()
{
	mIsLocked = false;
	mIsPressed = false;

	// ��ʼ��ħ������λ�ã���������Ĭ������ɫ
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			for (int k = 0; k < 3; ++k)
			{
				mCubes[i][j][k].pos = XMFLOAT3(-2.0f + 2.0f * i,
					-2.0f + 2.0f * j, -2.0f + 2.0f * k);
				mCubes[i][j][k].rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
				memset(mCubes[i][j][k].faceColors, 0, 
					sizeof mCubes[i][j][k].faceColors);
			}
	
	// +X��Ϊ��ɫ��-X��Ϊ��ɫ
	// +Y��Ϊ��ɫ��-Y��Ϊ��ɫ
	// +Z��Ϊ��ɫ��-Z��Ϊ��ɫ
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
		{
			mCubes[2][i][j].faceColors[RubikFace_PosX] = RubikFaceColor_Orange;
			mCubes[0][i][j].faceColors[RubikFace_NegX] = RubikFaceColor_Red;

			mCubes[j][2][i].faceColors[RubikFace_PosY] = RubikFaceColor_Green;
			mCubes[j][0][i].faceColors[RubikFace_NegY] = RubikFaceColor_Blue;

			mCubes[i][j][2].faceColors[RubikFace_PosZ] = RubikFaceColor_Yellow;
			mCubes[i][j][0].faceColors[RubikFace_NegZ] = RubikFaceColor_White;
		}	


}

void Rubik::Update(float dt)
{
	if (mIsLocked)
	{
		int finishCount = 0;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < 3; ++k)
				{
					// ��x��y, z������ת�Ƕ��𽥹�0
					// x��
					float dTheta = (signbit(mCubes[i][j][k].rotation.x) ? -1.0f : 1.0f) * dt * mRotationSpeed;
					if (fabs(mCubes[i][j][k].rotation.x) < fabs(dTheta))
					{
						mCubes[i][j][k].rotation.x = 0.0f;
						finishCount++;
					}
					else
					{
						mCubes[i][j][k].rotation.x -= dTheta;
					}
					// y��
					dTheta = (signbit(mCubes[i][j][k].rotation.y) ? -1.0f : 1.0f) * dt * mRotationSpeed;
					if (fabs(mCubes[i][j][k].rotation.y) < fabs(dTheta))
					{
						mCubes[i][j][k].rotation.y = 0.0f;
						finishCount++;
					}
					else
					{
						mCubes[i][j][k].rotation.y -= dTheta;
					}
					// z��
					dTheta = (signbit(mCubes[i][j][k].rotation.z) ? -1.0f : 1.0f) * dt * mRotationSpeed;
					if (fabs(mCubes[i][j][k].rotation.z) < fabs(dTheta))
					{
						mCubes[i][j][k].rotation.z = 0.0f;
						finishCount++;
					}
					else
					{
						mCubes[i][j][k].rotation.z -= dTheta;
					}
				}
			}
		}

		// ���з��鶼�����������ܽ���
		if (finishCount == 81)
			mIsLocked = false;
	}
}

void Rubik::Draw(ComPtr<ID3D11DeviceContext> deviceContext, BasicEffect& effect)
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				effect.SetWorldMatrix(mCubes[i][j][k].GetWorldMatrix());
				for (int face = 0; face < 6; ++face)
				{
					effect.SetTexIndex(mCubes[i][j][k].faceColors[face]);
					effect.Apply(deviceContext);
					deviceContext->DrawIndexed(6, 0, 4 * face);
				}
			}
		}
	}	
}

bool Rubik::IsLocked() const
{
	return mIsLocked;
}

bool Rubik::IsCompleted() const
{
	RubikFaceColor posX, negX, posY, negY, posZ, negZ;
	posX = mCubes[2][0][0].faceColors[0];
	negX = mCubes[0][0][0].faceColors[1];
	posY = mCubes[0][2][0].faceColors[2];
	negY = mCubes[0][0][0].faceColors[3];
	posZ = mCubes[0][0][2].faceColors[4];
	negZ = mCubes[0][0][0].faceColors[5];

	for (int j = 0; j < 3; ++j)
		for (int k = 0; k < 3; ++k)
			if (mCubes[2][j][k].faceColors[0] != posX || mCubes[0][j][k].faceColors[1] != negX)
				return false;

	for (int k = 0; k < 3; ++k)
		for (int i = 0; i < 3; ++i)
			if (mCubes[i][2][k].faceColors[2] != posY || mCubes[i][0][k].faceColors[3] != negY)
				return false;

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			if (mCubes[i][j][2].faceColors[4] != posZ || mCubes[i][j][0].faceColors[5] != negZ)
				return false;

	return true;
}

DirectX::XMINT3 Rubik::HitCube(Ray ray, float * pDist) const
{
	BoundingOrientedBox box(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	BoundingOrientedBox transformedBox;
	XMINT3 res = XMINT3(-1, -1, -1);
	float dist, minDist = FLT_MAX;

	// ����ʰȡ��¶�����������(ͬʱҲ�Ǿ�������������)
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				box.Transform(transformedBox, mCubes[i][j][k].GetWorldMatrix());
				if (ray.Hit(transformedBox, &dist) && dist < minDist)
				{
					minDist = dist;
					res = XMINT3(i, j, k);
				}
			}
		}
	}
	if (pDist)
		*pDist = (minDist == FLT_MAX ? 0.0f : minDist);
		
	return res;
}

void Rubik::RotateX(int pos, float dTheta, bool isPressed)
{
	if (!mIsLocked)
	{
		// ���鵱ǰ�Ƿ�Ϊ���̲���
		// ������Ϊ�������̲���ʱ�Ż��������ֵΪpi/2�ı���(������0)��˲ʱֵ
		bool isKeyOp =  static_cast<int>(round(dTheta / XM_PIDIV2)) != 0 &&
			(fabs(fmod(dTheta, XM_PIDIV2) < 1e-5f));
		// ������������������⣬�ܾ����̵Ĳ���
		if (mIsPressed && isKeyOp)
		{
			return;
		}

		mIsPressed = isPressed;

		// ������ת״̬
		for (int j = 0; j < 3; ++j)
			for (int k = 0; k < 3; ++k)
			{
				switch (pos)
				{
				case 3: mCubes[0][j][k].rotation.x += dTheta;
				case -2: mCubes[1][j][k].rotation.x += dTheta;
					mCubes[2][j][k].rotation.x += dTheta;
					break;
				case -1: mCubes[0][j][k].rotation.x += dTheta; 
					mCubes[1][j][k].rotation.x += dTheta; 
					break;
				
				default: mCubes[pos][j][k].rotation.x += dTheta;
				}
				
			}
				

		// ������̲������
		if (!mIsPressed)
		{
			// ��ʼ������ʾ״̬
			mIsLocked = true;

			// ����Ԥ��ת
			PreRotateX(isKeyOp);
		}
	}
}

void Rubik::RotateY(int pos, float dTheta, bool isPressed)
{
	if (!mIsLocked)
	{
		// ���鵱ǰ�Ƿ�Ϊ���̲���
		// ������Ϊ�������̲���ʱ�Ż��������ֵΪpi/2�ı���(������0)��˲ʱֵ
		bool isKeyOp = static_cast<int>(round(dTheta / XM_PIDIV2)) != 0 &&
			(fabs(fmod(dTheta, XM_PIDIV2) < 1e-5f));
		// ������������������⣬�ܾ����̵Ĳ���
		if (mIsPressed && isKeyOp)
		{
			return;
		}

		

		for (int k = 0; k < 3; ++k)
			for (int i = 0; i < 3; ++i)
			{
				switch (pos)
				{
				case 3: mCubes[i][0][k].rotation.y += dTheta;
				case -2: mCubes[i][1][k].rotation.y += dTheta;
					mCubes[i][2][k].rotation.y += dTheta;
					break;
				case -1: mCubes[i][0][k].rotation.y += dTheta;
					mCubes[i][1][k].rotation.y += dTheta;
					break;
				
				default: mCubes[i][pos][k].rotation.y += dTheta;
				}
			}

		mIsPressed = isPressed;

		// ������̲������
		if (!mIsPressed)
		{
			// ��ʼ������ʾ״̬
			mIsLocked = true;

			// ����Ԥ��ת
			PreRotateY(isKeyOp);
		}
	}
}

void Rubik::RotateZ(int pos, float dTheta, bool isPressed)
{
	if (!mIsLocked)
	{
		// ���鵱ǰ�Ƿ�Ϊ���̲���
		// ������Ϊ�������̲���ʱ�Ż��������ֵΪpi/2�ı���(������0)��˲ʱֵ
		bool isKeyOp = static_cast<int>(round(dTheta / XM_PIDIV2)) != 0 &&
			(fabs(fmod(dTheta, XM_PIDIV2) < 1e-5f));
		// ������������������⣬�ܾ����̵Ĳ���
		if (mIsPressed && isKeyOp)
		{
			return;
		}

		mIsPressed = isPressed;

		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
			{
				switch (pos)
				{
				case 3: mCubes[i][j][0].rotation.z += dTheta;
				case -2: mCubes[i][j][1].rotation.z += dTheta;
					mCubes[i][j][2].rotation.z += dTheta;
					break;
				case -1: mCubes[i][j][0].rotation.z += dTheta;
					mCubes[i][j][1].rotation.z += dTheta;
					break;
				
				default: mCubes[i][j][pos].rotation.z += dTheta;
				}
			}

				

		// ������̲������
		if (!mIsPressed)
		{
			// ��ʼ������ʾ״̬
			mIsLocked = true;

			// ����Ԥ��ת
			PreRotateZ(isKeyOp);
		}
	}
}

void Rubik::SetRotationSpeed(float rad)
{
	assert(rad > 0.0f);
	mRotationSpeed = rad;
}

ComPtr<ID3D11ShaderResourceView> Rubik::GetTexArray() const
{
	return mTexArray;
}

ComPtr<ID3D11ShaderResourceView> Rubik::CreateRubikCubeTextureArrayFromMemory(
	ComPtr<ID3D11Device> device,
	ComPtr<ID3D11DeviceContext> deviceContext)
{
	// ֻ���ļ�ȱʧ������Ż���������
	// ���ڴ洴��

	// ������������
	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = 128;
	texArrayDesc.Height = 128;
	texArrayDesc.MipLevels = 0;	// ָ������������mipmap��
	texArrayDesc.ArraySize = 7;
	texArrayDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texArrayDesc.SampleDesc.Count = 1;		// ��ʹ�ö��ز���
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;	// ����mipmap��Ҫ����ȾĿ��
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;	// ָ����Ҫ����mipmap

	ComPtr<ID3D11Texture2D> texArray;
	HR(device->CreateTexture2D(&texArrayDesc, nullptr, texArray.GetAddressOf()));
	// �����������ȡ�������������Ի�ȡ���ɵ�mipLevel
	texArray->GetDesc(&texArrayDesc);

	// (r, g, b, a)
	unsigned colors[7] = {
		'\x0\x0\x0\xff',		// ��ɫ
		'\xff\x6c\x0\xff',		// ��ɫ
		'\xdc\x42\x2f\xff',		// ��ɫ
		'\x0\x9d\x54\xff',		// ��ɫ
		'\x3d\x81\xf6\xff',		// ��ɫ
		'\xfd\xcc\x9\xff',		// ��ɫ
		'\xff\xff\xff\xff'		// ��ɫ
	};


	uint32_t textureMap[128][128];
	// Ĭ���ȴ�����ɫ
	for (int i = 0; i < 128; ++i)
		for (int j = 0; j < 128; ++j)
			textureMap[i][j] = colors[0];

	deviceContext->UpdateSubresource(texArray.Get(),
		D3D11CalcSubresource(0, 0, texArrayDesc.MipLevels),
		nullptr,
		textureMap, 
		128 * 4,
		128 * 128 * 4
	);
	// ����������ɫ������
	for (int i = 1; i <= 6; ++i)
	{
		for (int y = 7; y <= 17; ++y)
			for (int x = 25 - y; x <= 102 + y; ++x)
				textureMap[y][x] = textureMap[127 - y][x] = colors[i];

		for (int y = 18; y <= 109; ++y)
			for (int x = 7; x <= 120; ++x)
				textureMap[y][x] = colors[i];


		// ��������
		deviceContext->UpdateSubresource(texArray.Get(),
			D3D11CalcSubresource(0, i, texArrayDesc.MipLevels),
			nullptr,
			textureMap,
			128 * 4,
			128 * 128 * 4
		);

	}
	// �������������SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = -1;	// ����mipamp
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = 7;

	ComPtr<ID3D11ShaderResourceView> texArraySRV;
	HR(device->CreateShaderResourceView(texArray.Get(), &viewDesc, texArraySRV.GetAddressOf()));
	// ����mipmap
	deviceContext->GenerateMips(texArraySRV.Get());
	return texArraySRV;
}

void Rubik::PreRotateX(bool isKeyOp)
{
	for (int i = 0; i < 3; ++i)
	{
		// ��ǰ��û����ת��ֱ������
		if (fabs(mCubes[i][0][0].rotation.x) < 1e-5f)
			continue;
		// ���ڴ�ʱ����ת������з�����ת�Ƕȶ���һ���ģ����Դ���ȡһ�������㡣
		// �����λ��[-pi/4, pi/4)������Ҫ˳ʱ����ת90�ȵĴ���
		int times = static_cast<int>(round(mCubes[i][0][0].rotation.x / XM_PIDIV2));
		// ����λ����ӳ�䵽[0, 3]���Լ�����С����˳ʱ����ת90�ȵĴ���
		int minTimes = (times % 4 + 4) % 4;

		// �������б���ת����ĳ�ʼ�Ƕ�
		for (int j = 0; j < 3; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				// ���̰��º�ı仯
				if (isKeyOp)
				{
					// ˳ʱ����ת90��--->ʵ�������-90�ȼӵ�0��
					// ��ʱ����ת90��--->ʵ�������90�ȼ���0��
					mCubes[i][j][k].rotation.x *= -1.0f;
				}
				// ����ͷź�ı仯
				else
				{
					// ��λ��[-pi/4, pi/4)������
					mCubes[i][j][k].rotation.x -= times * XM_PIDIV2;
				}
			}
		}

		std::vector<XMINT2> indices1, indices2;
		GetSwapIndexArray(minTimes, indices1, indices2);
		size_t swapTimes = indices1.size();
		for (size_t idx = 0; idx < swapTimes; ++idx)
		{
			// �������������尴���������Ľ���
			XMINT2 srcIndex = indices1[idx];
			XMINT2 targetIndex = indices2[idx];
			// ��Ϊ2��˳ʱ����ת����ֻ��4�ζԽǵ���
			// ������Ҫ6���ڽ�(��)�Ի�
			for (int face = 0; face < 6; ++face)
			{
				std::swap(mCubes[i][srcIndex.x][srcIndex.y].faceColors[face],
					mCubes[i][targetIndex.x][targetIndex.y].faceColors[
						GetTargetSwapFaceRotationX(static_cast<RubikFace>(face), minTimes)]);
			}
		}
	}
}

void Rubik::PreRotateY(bool isKeyOp)
{
	for (int j = 0; j < 3; ++j)
	{
		// ��ǰ��û����ת��ֱ������
		if (fabs(mCubes[0][j][0].rotation.y) < 1e-5f)
			continue;
		// ���ڴ�ʱ����ת������з�����ת�Ƕȶ���һ���ģ����Դ���ȡһ�������㡣
		// �����λ��[-pi/4, pi/4)������Ҫ˳ʱ����ת90�ȵĴ���
		int times = static_cast<int>(round(mCubes[0][j][0].rotation.y / XM_PIDIV2));
		// ����λ����ӳ�䵽[0, 3]���Լ�����С����˳ʱ����ת90�ȵĴ���
		int minTimes = (times % 4 + 4) % 4;

		// �������б���ת����ĳ�ʼ�Ƕ�
		for (int k = 0; k < 3; ++k)
		{
			for (int i = 0; i < 3; ++i)
			{
				// ������Ϊ�������̲���ʱ�Ż��������ֵΪpi/2��˲ʱֵ
				// ���̰��º�ı仯
				if (isKeyOp)
				{
					// ˳ʱ����ת90��--->ʵ�������-90�ȼӵ�0��
					// ��ʱ����ת90��--->ʵ�������90�ȼ���0��
					mCubes[i][j][k].rotation.y *= -1.0f;
				}
				// ����ͷź�ı仯
				else
				{
					// ��λ��[-pi/4, pi/4)������
					mCubes[i][j][k].rotation.y -= times * XM_PIDIV2;
				}
			}
		}

		std::vector<XMINT2> indices1, indices2;
		GetSwapIndexArray(minTimes, indices1, indices2);
		size_t swapTimes = indices1.size();
		for (size_t idx = 0; idx < swapTimes; ++idx)
		{
			// �������������尴���������Ľ���
			XMINT2 srcIndex = indices1[idx];
			XMINT2 targetIndex = indices2[idx];
			// ��Ϊ2��˳ʱ����ת����ֻ��4�ζԽǵ���
			// ������Ҫ6���ڽ�(��)�Ի�
			for (int face = 0; face < 6; ++face)
			{
				std::swap(mCubes[srcIndex.y][j][srcIndex.x].faceColors[face],
					mCubes[targetIndex.y][j][targetIndex.x].faceColors[
						GetTargetSwapFaceRotationY(static_cast<RubikFace>(face), minTimes)]);
			}
		}
	}
}

void Rubik::PreRotateZ(bool isKeyOp)
{
	for (int k = 0; k < 3; ++k)
	{
		// ��ǰ��û����ת��ֱ������
		if (fabs(mCubes[0][0][k].rotation.z) < 1e-5f)
			continue;

		// ���ڴ�ʱ����ת������з�����ת�Ƕȶ���һ���ģ����Դ���ȡһ�������㡣
		// �����λ��[-pi/4, pi/4)������Ҫ˳ʱ����ת90�ȵĴ���
		int times = static_cast<int>(round(mCubes[0][0][k].rotation.z / XM_PIDIV2));
		// ����λ����ӳ�䵽[0, 3]���Լ�����С����˳ʱ����ת90�ȵĴ���
		int minTimes = (times % 4 + 4) % 4;

		// �������б���ת����ĳ�ʼ�Ƕ�
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				// ������Ϊ�������̲���ʱ�Ż��������ֵΪpi/2��˲ʱֵ
				// ���̰��º�ı仯
				if (isKeyOp)
				{
					// ˳ʱ����ת90��--->ʵ�������-90�ȼӵ�0��
					// ��ʱ����ת90��--->ʵ�������90�ȼ���0��
					mCubes[i][j][k].rotation.z *= -1.0f;
				}
				// ����ͷź�ı仯
				else
				{
					// ��λ��[-pi/4, pi/4)������
					mCubes[i][j][k].rotation.z -= times * XM_PIDIV2;
				}
			}
		}

		std::vector<XMINT2> indices1, indices2;
		GetSwapIndexArray(minTimes, indices1, indices2);
		size_t swapTimes = indices1.size();
		for (size_t idx = 0; idx < swapTimes; ++idx)
		{
			// �������������尴���������Ľ���
			XMINT2 srcIndex = indices1[idx];
			XMINT2 targetIndex = indices2[idx];
			// ��Ϊ2��˳ʱ����ת����ֻ��4�ζԽǵ���
			// ������Ҫ6���ڽ�(��)�Ի�
			for (int face = 0; face < 6; ++face)
			{
				std::swap(mCubes[srcIndex.x][srcIndex.y][k].faceColors[face],
					mCubes[targetIndex.x][targetIndex.y][k].faceColors[
						GetTargetSwapFaceRotationZ(static_cast<RubikFace>(face), minTimes)]);
			}
		}
	}
}

void Rubik::GetSwapIndexArray(int minTimes, std::vector<DirectX::XMINT2>& outArr1, std::vector<DirectX::XMINT2>& outArr2) const
{
	// ����һ��˳ʱ��90����ת�൱��ʱ�뽻��6��(���Ǻ����3��)
	// 1   2   4   2   4   2   4   1
	//   *   ->  *   ->  *   ->  *
	// 4   3   1   3   3   1   3   2
	if (minTimes == 1)
	{
		outArr1 = { XMINT2(0, 0), XMINT2(0, 1), XMINT2(0, 2), XMINT2(1, 2), XMINT2(2, 2), XMINT2(2, 1) };
		outArr2 = { XMINT2(0, 2), XMINT2(1, 2), XMINT2(2, 2), XMINT2(2, 1), XMINT2(2, 0), XMINT2(1, 0) };
	}
	// ����һ��˳ʱ��90����ת�൱��ʱ�뽻��4��(���Ǻ����2��)
	// 1   2   3   2   3   4
	//   *   ->  *   ->  *  
	// 4   3   4   1   2   1
	else if (minTimes == 2)
	{
		outArr1 = { XMINT2(0, 0), XMINT2(0, 1), XMINT2(0, 2), XMINT2(1, 2) };
		outArr2 = { XMINT2(2, 2), XMINT2(2, 1), XMINT2(2, 0), XMINT2(1, 0) };
	}
	// ����һ��˳ʱ��90����ת�൱��ʱ�뽻��6��(���Ǻ����3��)
	// 1   2   4   2   4   2   4   1
	//   *   ->  *   ->  *   ->  *
	// 4   3   1   3   3   1   3   2
	else if (minTimes == 3)
	{
		outArr1 = { XMINT2(0, 0), XMINT2(1, 0), XMINT2(2, 0), XMINT2(2, 1), XMINT2(2, 2), XMINT2(1, 2) };
		outArr2 = { XMINT2(2, 0), XMINT2(2, 1), XMINT2(2, 2), XMINT2(1, 2), XMINT2(0, 2), XMINT2(0, 1) };
	}
	// 0��˳ʱ����ת���䣬�����쳣��ֵҲ����
	else
	{
		outArr1.clear();
		outArr2.clear();
	}
	
}

RubikFace Rubik::GetTargetSwapFaceRotationX(RubikFace face, int times) const
{
	if (face == RubikFace_PosX || face == RubikFace_NegX)
		return face;
	while (times--)
	{
		switch (face)
		{
		case RubikFace_PosY: face = RubikFace_NegZ; break;
		case RubikFace_PosZ: face = RubikFace_PosY; break;
		case RubikFace_NegY: face = RubikFace_PosZ; break;
		case RubikFace_NegZ: face = RubikFace_NegY; break;
		}
	}
	return face;
}

RubikFace Rubik::GetTargetSwapFaceRotationY(RubikFace face, int times) const
{
	if (face == RubikFace_PosY || face == RubikFace_NegY)
		return face;
	while (times--)
	{
		switch (face)
		{
		case RubikFace_PosZ: face = RubikFace_NegX; break;
		case RubikFace_PosX: face = RubikFace_PosZ; break;
		case RubikFace_NegZ: face = RubikFace_PosX; break;
		case RubikFace_NegX: face = RubikFace_NegZ; break;
		}
	}
	return face;
}

RubikFace Rubik::GetTargetSwapFaceRotationZ(RubikFace face, int times) const
{
	if (face == RubikFace_PosZ || face == RubikFace_NegZ)
		return face;
	while (times--)
	{
		switch (face)
		{
		case RubikFace_PosX: face = RubikFace_NegY; break;
		case RubikFace_PosY: face = RubikFace_PosX; break;
		case RubikFace_NegX: face = RubikFace_PosY; break;
		case RubikFace_NegY: face = RubikFace_NegX; break;
		}
	}
	return face;
}
