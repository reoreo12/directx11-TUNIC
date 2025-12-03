#include "Collider.h"
#include "GameInstance.h"
#include "GameObject.h"

CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& Prototype)
	: CComponent { Prototype }
	, m_eColliderType { Prototype.m_eColliderType }
#ifdef _DEBUG
	, m_pBatch { Prototype.m_pBatch }
	, m_pEffect { Prototype.m_pEffect }
	, m_pInputLayout { Prototype.m_pInputLayout }
#endif
{
#ifdef _DEBUG
	Safe_AddRef(m_pInputLayout);
#endif
}

HRESULT CCollider::Initialize_Prototype(TYPE eColliderType)
{
	m_eColliderType = eColliderType;

#ifdef _DEBUG
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);
	m_pEffect = new BasicEffect(m_pDevice);

	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode = { nullptr };
	size_t iLength = {};

	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iLength);

	m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iLength, &m_pInputLayout);
#endif

	return S_OK;
}

HRESULT CCollider::Initialize(void* pArg)
{
	const CBounding::BOUNDING_DESC* pDesc = static_cast<const CBounding::BOUNDING_DESC*>(pArg);

	switch (m_eColliderType)
	{
	case TYPE_AABB:
		m_pBounding = CBounding_AABB::Create(m_pDevice, m_pContext, pDesc);
		break;

	case TYPE_SPHERE:
		m_pBounding = CBounding_Sphere::Create(m_pDevice, m_pContext, pDesc);
		break;
	
	case TYPE_OBB:
		m_pBounding = CBounding_OBB::Create(m_pDevice, m_pContext, pDesc);
		break;
	}

	return S_OK;
}

void CCollider::Update(_fmatrix WorldMatrix)
{
	m_pBounding->Update(WorldMatrix);
}

_bool CCollider::Intersect(CCollider* pTargetCollider)
{
	return m_pBounding->Intersect(pTargetCollider->m_eColliderType, pTargetCollider->m_pBounding);
}

void CCollider::Update_CollisionState(CCollider* pTargetCollider)
{
	if (!pTargetCollider || !m_pOwner || !pTargetCollider->Get_Owner())
		return;

	// 죽은 오브젝트면 충돌 처리 스킵
	if (m_pOwner->Is_Dead() || pTargetCollider->Get_Owner()->Is_Dead())
		return;

	// 충돌 검사 결과가 true라면
	if (Intersect(pTargetCollider))
	{
		m_bIsColl = true;
		// OnCollisionEnter, Stay, Exit: 충돌 이벤트 콜백 함수
		// 처음 충돌했다면
		if (m_CollidingWith.insert(pTargetCollider).second)
			m_pOwner->OnCollisionEnter(this, pTargetCollider);
		// 이미 충돌 상태였다면
		else
			m_pOwner->OnCollisionStay(this, pTargetCollider);
	}
	// 충돌 검사 결과가 false라면
	else
	{
		// 충돌 중이었는데 해제됐다면
		if (m_CollidingWith.erase(pTargetCollider))
			m_pOwner->OnCollisionExit(this, pTargetCollider);
	}
}

#ifdef _DEBUG
HRESULT CCollider::Render()
{
	m_pContext->GSSetShader(nullptr, nullptr, 0);

	m_pContext->IASetInputLayout(m_pInputLayout);

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW));
	m_pEffect->SetProjection(m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ));

	m_pEffect->Apply(m_pContext);

	m_pBatch->Begin();

	m_pBounding->Render(m_pBatch, false == m_bIsColl ? XMVectorSet(0.f, 1.f, 0.f, 1.f) : XMVectorSet(1.f, 0.f, 0.f, 1.f));

	m_pBatch->End();

	return S_OK;
}
#endif

CCollider* CCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eColliderType)
{
	CCollider* pInstance = new CCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eColliderType)))
	{
		MSG_BOX("Failed To Created : CCollider");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CCollider::Clone(void* pArg)
{
	CCollider* pInstance = new CCollider(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CCollider");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCollider::Free()
{
	__super::Free();

	Safe_Release(m_pBounding);

	for (CCollider* pOther : m_CollidingWith)
		pOther->m_CollidingWith.erase(this);

	Clear_CollisionState();

#ifdef _DEBUG
	if (!m_bIsCloned)
	{
		Safe_Delete(m_pBatch);
		Safe_Delete(m_pEffect);
	}

	Safe_Release(m_pInputLayout);
#endif
}
