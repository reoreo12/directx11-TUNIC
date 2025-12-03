#pragma once
#include "Base.h"
#include "Tool_Defines.h"

BEGIN(Engine)
class CGameInstance;
class CGameObject;
class CModel;
class CImGui_Manager;
END

BEGIN(Tool)

class CNavigationTool : public CBase
{
private:
	CNavigationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CNavigationTool() = default;

public:
	HRESULT								Initialize();

public:
	const vector<array<_float3, 3>>& Get_Cells() const { return m_vecCells; }
	const vector<_int>& Get_CellFlags() const { return m_vecCellFlags; }

	void								Render_ImGui();
	void								Render_Cells_Points();

private:
	// 툴에 필요한 ImGui 패널 렌더링
	void								Draw_NavigationWindow();
	// .dat 파일 리스트 + Load 기능 렌더링
	void								Draw_FileListSection(); 
	// 마우스 피킹 처리 함수
	void								Handle_MousePicking();
	// 셀 리스트/삭제/플래그/저장 기능 렌더링
	void								Draw_CellList_Controls();

	// 피킹 보정(스냅) 함수
	_float3								Get_SnappedPoint(_float3 newPoint) const;
	// 셀을 구성하는 세 개의 정점을 시계 방향으로 정렬하는 함수
	array<_float3, 3>					Sort_TriangleClockwise(const _float3& p1, const _float3& p2, const _float3& p3) const;

	// 피킹 정점 삭제 함수
	void								Remove_PickedPoint();
	// 내비게이션 데이터 파일 저장 함수
	void								Save_NavigationData(const _string& strFilename);
	// 내비게이션 데이터 파일 로드 함수
	void								Load_NavigationData(const _string& strFilename);

	// 파일 리스트 생성/해제
	HRESULT								Ready_NavFileList();

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };

	CGameInstance*				m_pGameInstance = { nullptr };

private:
	// 피킹 정점들
	vector<_float3>				m_vecPickedPoints;
	// 생성된 삼각형 셀들(시계 방향)
	vector<array<_float3, 3>>	m_vecCells;	
	// 셀별 플래그 값
	vector<_int>				m_vecCellFlags;
	_int						m_iSelectedCellIndex = { -1 };

	// 피킹 정점 스냅 On/Off
	_bool						m_bEnableSnap = { true };
	// 스냅 거리
	_float						m_fSnapRange = { 0.5f };
	// 오브젝트 피킹/셀 피킹 모드 선택
	_bool						m_bPickToSelect = { false };

	// 파일 리스트
	vector<_string>				m_vecNavFileList;
	_bool						m_bNavFileListLoaded = { false };
	_int						m_iSelectedNavFile = { 0 };

public:
	static CNavigationTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Free() override;
};

END