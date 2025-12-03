#include "NavigationTool.h"
#include "GameInstance.h"
#include "ImGui_Manager.h"
#include "ImGui/imgui.h"
#include "Cell.h"

#include <fstream>
#include <sstream>
#include <io.h>

#define ACCESS _access
#define FILE_EXISTS(path) (ACCESS(path, 0) == 0)

CNavigationTool::CNavigationTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CNavigationTool::Initialize()
{
    m_bEnableSnap = true;
    m_fSnapRange = 0.5f;
    m_bPickToSelect = false;
    m_iSelectedCellIndex = -1;

	return Ready_NavFileList();
}

HRESULT CNavigationTool::Ready_NavFileList()
{
    if (m_bNavFileListLoaded)
        return S_OK;

    const _char* szPath = "../../Tool/Bin/DataFiles/*.dat";
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(szPath, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                m_vecNavFileList.push_back(findFileData.cFileName);
        } while (FindNextFileA(hFind, &findFileData) != 0);
       
        FindClose(hFind);
    }

    m_bNavFileListLoaded = true;
    m_iSelectedNavFile = 0;

    return S_OK;
}

void CNavigationTool::Render_ImGui()
{
    Draw_NavigationWindow();
}

void CNavigationTool::Render_Cells_Points()
{
    if (m_vecCells.empty() && m_vecPickedPoints.empty())
        return;

    // IA(입력 어셈블러) 설정 - 삼각형 리스트
    m_pGameInstance->Set_DebugIA(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Rasterizer 및 Blend 설정
    m_pGameInstance->Set_DebugRSState(RS_WIREFRAME_NONE);
    m_pGameInstance->Set_DebugBlendState(BS_BLEND, _float4(1.f, 1.f, 1.f, 1.f));

    // 행렬 설정
    _matrix WorldMat = XMMatrixIdentity();
    _matrix ViewMat = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
    _matrix ProjMat = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);

    BasicEffect* pEffect = m_pGameInstance->Get_DebugEffect();
    if (!pEffect)
        return;

    pEffect->SetWorld(WorldMat);
    pEffect->SetView(ViewMat);
    pEffect->SetProjection(ProjMat);

    auto pBatch = m_pGameInstance->Get_PrimitiveBatch();

    // 렌더링 시작
    m_pGameInstance->Begin_DebugRender();

    // 정점, 셀 색상 설정
    _vector vDefaultColor = XMVectorSet(0.f, 1.f, 0.f, 0.8f);    // 진한 초록색(기본 셀)
    _vector vSelectedColor = XMVectorSet(1.f, 0.f, 0.f, 1.f);    // 빨강(선택된 셀)
    _vector vPointColor = XMVectorSet(1.f, 1.f, 0.f, 1.f);       // 노란색(피킹 정점)
    _vector vLadderTopColor = XMVectorSet(0.f, 1.f, 1.f, 1.f);    // 하늘(LADDER_TOP)
    _vector vLadderMidColor = XMVectorSet(0.f, 0.4f, 1.f, 1.f);   // 파랑(LADDER_MIDDLE)
    _vector vLadderBotColor = XMVectorSet(0.6f, 0.f, 0.6f, 1.f);  // 보라(LADDER_BOTTOM)

    // 기본 셀(초록색) 우선 렌더링
    for (size_t i = 0; i < m_vecCells.size(); ++i)
    {
        if (static_cast<_int>(i) == m_iSelectedCellIndex || m_vecCellFlags[i] != 0)
            continue;

        pBatch->DrawTriangle(
            VertexPositionColor(XMLoadFloat3(&m_vecCells[i][0]), vDefaultColor),
            VertexPositionColor(XMLoadFloat3(&m_vecCells[i][1]), vDefaultColor),
            VertexPositionColor(XMLoadFloat3(&m_vecCells[i][2]), vDefaultColor)
        );
    }

    // 플래그가 있는 셀 렌더링(하늘/파랑/보라)
    for (size_t i = 0; i < m_vecCells.size(); ++i)
    {
        if (i == m_iSelectedCellIndex || m_vecCellFlags[i] == 0)
            continue;

        _vector vDrawColor = vDefaultColor; // 초기 값

        switch (m_vecCellFlags[i])
        {
        case 1:
            vDrawColor = vLadderTopColor; // 하늘
            break;
        case 2:
           vDrawColor = vLadderMidColor; // 파랑
            break;
        case 3:
            vDrawColor = vLadderBotColor; // 보라
            break;
        default:
            vDrawColor = vDefaultColor; // 예외 처리
            break;
        }

        pBatch->DrawTriangle(
            VertexPositionColor(XMLoadFloat3(&m_vecCells[i][0]), vDrawColor),
            VertexPositionColor(XMLoadFloat3(&m_vecCells[i][1]), vDrawColor),
            VertexPositionColor(XMLoadFloat3(&m_vecCells[i][2]), vDrawColor)
        );
    }

    // 이후 선택한 셀을 렌더링(먼저 렌더링할 경우 기본 셀 색에 가려지기 때문)
    if (m_iSelectedCellIndex >= 0 && m_iSelectedCellIndex < static_cast<_int>(m_vecCells.size()))
    {
        const auto& selectedTriangle = m_vecCells[m_iSelectedCellIndex];

        pBatch->DrawTriangle(
            VertexPositionColor(XMLoadFloat3(&selectedTriangle[0]), vSelectedColor),
            VertexPositionColor(XMLoadFloat3(&selectedTriangle[1]), vSelectedColor),
            VertexPositionColor(XMLoadFloat3(&selectedTriangle[2]), vSelectedColor)
        );
    }

    // 현재 선택 중인 피킹 좌표를 사각형으로 표시
    if (!m_vecPickedPoints.empty())
    {
        for (const auto& point : m_vecPickedPoints)
        {
            _vector vPointPos = XMLoadFloat3(&point);

            _float fPointSize = 0.2f;

            _vector vP1 = XMVectorSetW(vPointPos + XMVectorSet(-fPointSize, fPointSize, 0.f, 0.f), 1.f);
            _vector vP2 = XMVectorSetW(vPointPos + XMVectorSet(fPointSize, fPointSize, 0.f, 0.f), 1.f);
            _vector vP3 = XMVectorSetW(vPointPos + XMVectorSet(fPointSize, -fPointSize, 0.f, 0.f), 1.f);
            _vector vP4 = XMVectorSetW(vPointPos + XMVectorSet(-fPointSize, -fPointSize, 0.f, 0.f), 1.f);

            pBatch->DrawLine(VertexPositionColor(vP1, vPointColor), VertexPositionColor(vP2, vPointColor));
            pBatch->DrawLine(VertexPositionColor(vP2, vPointColor), VertexPositionColor(vP3, vPointColor));
            pBatch->DrawLine(VertexPositionColor(vP3, vPointColor), VertexPositionColor(vP4, vPointColor));
            pBatch->DrawLine(VertexPositionColor(vP4, vPointColor), VertexPositionColor(vP1, vPointColor));
        }
    }

    // 렌더링 종료
    m_pGameInstance->End_DebugRender();
}

void CNavigationTool::Draw_NavigationWindow()
{
    ImGui::Begin("Navigation Tool");

    ImGui::Text("Navigation Tool - Pick Mesh");

    Draw_FileListSection();

    ImGui::Separator();

    Handle_MousePicking();
    Draw_CellList_Controls();

    ImGui::End();
}

void CNavigationTool::Draw_FileListSection()
{
    // 저장된 .dat 파일 리스트
    ImGui::Text("Saved Navigation Files:");

    if (!m_vecNavFileList.empty())
    {
        if (ImGui::BeginListBox("##NavDatFiles", ImVec2(-FLT_MIN, 100)))
        {
            for (size_t i = 0; i < m_vecNavFileList.size(); i++)
            {
                _bool bIsSelected = (m_iSelectedNavFile == i);
                if (ImGui::Selectable(m_vecNavFileList[i].c_str(), bIsSelected))
                    m_iSelectedNavFile = static_cast<_int>(i);
            }

            ImGui::EndListBox();
        }
    }

    else
    {
        ImGui::Text("No .dat files found.");
    }

    // 리스트 박스에서 선택한 .dat 파일 불러오기
    if (ImGui::Button("Load Selected Nav Data"))
    {
        if (!m_vecNavFileList.empty())
        {
            _string strSelectedFile = "../../Tool/Bin/DataFiles/" + m_vecNavFileList[m_iSelectedNavFile];
            
            Load_NavigationData(strSelectedFile);
        }
    }
}

void CNavigationTool::Handle_MousePicking()
{
    // ImGui 위젯 위에 마우스 커서가 있으면 피킹은 비활성화
    _bool bOnWidget = ImGui::GetIO().WantCaptureMouse;
    if (bOnWidget)
        return;

    if (!m_pGameInstance->Mouse_Down(MOUSEKEYSTATE::DIM_LB))
        return;

    _vector vRayOrigin, vRayDir;

    // 피킹 여부 검사
    if (!m_pGameInstance->Calculate_PickingRay(vRayOrigin, vRayDir)) // 화면 좌표에서 3D Ray 생성
        return;
    
    // 셀 피킹 모드(셀 선택)
    if (m_bPickToSelect)
    {
        for (size_t i = 0; i < m_vecCells.size(); ++i)
        {
            const auto& cell = m_vecCells[i];

            _float3 vP0 = cell[0];
            _float3 vP1 = cell[1];
            _float3 vP2 = cell[2];

            _float fT, fU, fV;

            if (m_pGameInstance->Ray_TriangleIntersection(vRayOrigin, vRayDir, vP0, vP1, vP2, fT, fU, fV))
            {
                m_iSelectedCellIndex = static_cast<_int>(i);
                break;
            }
        }

        return;
    }

    // 오브젝트 피킹 모드(정점 찍기)
    // 현재 툴 프로젝트에 생성된 오브젝트 목록을 가져온다
    const auto& createdObjects = m_pGameInstance->Get_Objects();

    // Ray와 충돌한 지점 중 가장 가까운 거리를 저장할 변수
    _float fClosestDist = FLT_MAX;
    // 가장 가까운 피킹 좌표
    _float3 vClosestPoint;
    // 피킹 여부
    _bool bHit = false;

    for (const auto& pair : createdObjects)
    {
        CGameObject* pObj = pair.second;
        if (!pObj)
            continue;

        CModel* pModel = pObj->Get_Component<CModel>(L"Com_Model");

        if (!pModel)
            continue;

        _float3 vHitPoint;
        _float fDist = FLT_MAX;

        // 피킹 검사
        if (pModel->Picking_Mesh(vRayOrigin, vRayDir, vHitPoint, fDist))
        {
            // 가장 가까운 지점인지 비교
            if (fDist < fClosestDist)
            {
                fClosestDist = fDist;
                vClosestPoint = vHitPoint;
                bHit = true;
            }
        }
    }

    if (!bHit)
        return;

    // 정점 자동 보정 스냅
    vClosestPoint = Get_SnappedPoint(vClosestPoint);

    m_vecPickedPoints.push_back(vClosestPoint);

    if (m_vecPickedPoints.size() % 3 == 0)
    {
        const _float3& vP1 = m_vecPickedPoints[m_vecPickedPoints.size() - 3];
        const _float3& vP2 = m_vecPickedPoints[m_vecPickedPoints.size() - 2];
        const _float3& vP3 = m_vecPickedPoints[m_vecPickedPoints.size() - 1];

        m_vecCells.push_back(Sort_TriangleClockwise(vP1, vP2, vP3));
        m_vecCellFlags.push_back(0); // 초기 값: 기본 플래그
    }
}

void CNavigationTool::Draw_CellList_Controls()
{
    if (m_vecCells.empty())
        return;

    ImGui::Text("Created Cells");

    // 셀 리스트
    if (ImGui::BeginListBox("##CellList", ImVec2(-FLT_MIN, 150)))
    {
        for (size_t i = 0; i < m_vecCells.size(); ++i)
        {
            const auto& cell = m_vecCells[i];
            _bool bIsSelected = (m_iSelectedCellIndex == static_cast<_int>(i));

            _char szCellInfo[128] = {};
            sprintf_s(szCellInfo,
                "Cell %zu | Flag: %d | P1: (%.2f, %.2f, %.2f) P2: (%.2f, %.2f, %.2f) P3: (%.2f, %.2f, %.2f)",
                i,
                m_vecCellFlags[i],
                cell[0].x, cell[0].y, cell[0].z,
                cell[1].x, cell[1].y, cell[1].z,
                cell[2].x, cell[2].y, cell[2].z);

            // 선택 가능한 리스트 박스
            if (ImGui::Selectable(szCellInfo, bIsSelected))
                m_iSelectedCellIndex = static_cast<_int>(i);

            // 선택한 셀이 있는 지점으로 리스트 박스 자동 스크롤링
            if (bIsSelected)
                ImGui::SetScrollHereY();
        }

        ImGui::EndListBox();
    }

    // 선택된 셀 삭제 버튼
    if (m_iSelectedCellIndex >= 0 &&
        m_iSelectedCellIndex < static_cast<_int>(m_vecCells.size()))
    {
        if (ImGui::Button("Delete Selected Cell"))
        {
            // 선택된 셀의 세 정점
            _float3 vP1 = m_vecCells[m_iSelectedCellIndex][0];
            _float3 vP2 = m_vecCells[m_iSelectedCellIndex][1];
            _float3 vP3 = m_vecCells[m_iSelectedCellIndex][2];

            // 선택한 셀 삭제
            m_vecCells.erase(m_vecCells.begin() + m_iSelectedCellIndex);

            // 셀의 플래그도 삭제
            if (m_iSelectedCellIndex < static_cast<_int>(m_vecCellFlags.size()))
                m_vecCellFlags.erase(m_vecCellFlags.begin() + m_iSelectedCellIndex);

            // m_vecPickedPoints에서 사용되지 않는 정점 삭제
            auto isVertexUsed = [&](const _float3& vertex) -> _bool
                {
                    for (const auto& cell : m_vecCells)
                    {
                        if (XMVector3Equal(XMLoadFloat3(&cell[0]), XMLoadFloat3(&vertex)) ||
                            XMVector3Equal(XMLoadFloat3(&cell[1]), XMLoadFloat3(&vertex)) ||
                            XMVector3Equal(XMLoadFloat3(&cell[2]), XMLoadFloat3(&vertex)))
                            return true;
                    }

                    return false;
                };

            m_vecPickedPoints.erase(
                remove_if(m_vecPickedPoints.begin(), m_vecPickedPoints.end(),
                    [&](const _float3& point) { return (!isVertexUsed(point)); }),
                m_vecPickedPoints.end());

            // 선택한 인덱스 초기화
            m_iSelectedCellIndex = -1;
        }
    }

    // 모든 셀 삭제 버튼
    if (ImGui::Button("Delete All Cells"))
    {
        m_vecCells.clear();
        m_vecCellFlags.clear();
        m_vecPickedPoints.clear();
        m_iSelectedCellIndex = -1;
    }

    // 셀 플래그 설정
    if (m_iSelectedCellIndex >= 0 &&
        m_iSelectedCellIndex < static_cast<_int>(m_vecCells.size()))
    {
        ImGui::Text("Selected Cell Flag:");

        static const _char* flagNames[] =
        {
            "Default",          // 기본
            "Ladder_Top",       // 사다리 위
            "Ladder_Middle",    // 사다리
            "Ladder_Bottom",    // 사다리 아래
            "East_To_Boss",     // EastForest -> BossMap 맵 이동 셀
            "Boss_Start"        // 보스전 입구 셀
        };

        _int& flagRef = m_vecCellFlags[m_iSelectedCellIndex];
        _int iCurrentFlag = flagRef;

        if (ImGui::Combo("##CellFlag", &iCurrentFlag, flagNames, CCell::FLAG_END))
        {
            flagRef = iCurrentFlag; // 단일 플래그만 저장
        }
    }

    // 피킹 모드 전환 체크박스
    ImGui::Checkbox("Pick to Select Cell", &m_bPickToSelect);

    // 내비게이션 데이터 저장 버튼
    if (ImGui::Button("Save Navigation Data"))
    {
        // 리스트 박스에 나열된 모든 오브젝트를 파일에 저장
        _int iIndex = 0;
        _string strFileName;
        do {
            strFileName = "../../Tool/Bin/DataFiles/NavigationData";
            if (iIndex > 0) strFileName += "_" + to_string(iIndex);
            strFileName += ".dat";
            ++iIndex;
        } while (FILE_EXISTS(strFileName)); // 같은 이름의 파일이 존재하면 맨 뒤 인덱스만 증가

        Save_NavigationData(strFileName);
    }
}

_float3 CNavigationTool::Get_SnappedPoint(_float3 newPoint) const
{
    // 보정 비활성화 시 종료
    if (!m_bEnableSnap)
        return newPoint;

    for (const auto& existingPoint : m_vecPickedPoints)
    {
        _float fDist = sqrt(pow(existingPoint.x - newPoint.x, 2) +
                          pow(existingPoint.y - newPoint.y, 2) +
                          pow(existingPoint.z - newPoint.z, 2));

        // 기존 정점 중 0.5f만큼 가까운 정점이 있다면 보정
        if (fDist < m_fSnapRange)
            return existingPoint;
    }

    return newPoint;
}

array<_float3, 3> CNavigationTool::Sort_TriangleClockwise(const _float3& p1, const _float3& p2, const _float3& p3) const
{
    // 두 선분과 법선을 구해 시계 방향으로 정렬된 세 정점을 반환
    _vector v1 = XMLoadFloat3(&p2) - XMLoadFloat3(&p1);
    _vector v2 = XMLoadFloat3(&p3) - XMLoadFloat3(&p1);
    _vector vNormal = XMVector3Cross(v1, v2);

    if (XMVectorGetY(vNormal) < 0)
        return { p1, p3, p2 };

    return { p1, p2, p3 };
}

void CNavigationTool::Remove_PickedPoint()
{
    if (!m_vecPickedPoints.empty())
        m_vecPickedPoints.pop_back();
}

void CNavigationTool::Save_NavigationData(const _string& strFilename)
{
    ofstream outFile(strFilename, ios::binary);
    
    if (!outFile.is_open())
    {
        MessageBox(0, L"Failed to open file for saving navigation data!", L"Failed", MB_OK);
        return;
    }

    // 셀과 플래그 저장
    for (size_t i = 0; i < m_vecCells.size(); ++i)
    {
        _float3 vPoints[3] =
        {
            m_vecCells[i][0],
            m_vecCells[i][1],
            m_vecCells[i][2]
        };

        outFile.write(reinterpret_cast<const _char*>(vPoints), sizeof(_float3) * 3);

        _int iFlag = (i < m_vecCellFlags.size()) ? m_vecCellFlags[i] : 0; // 기본값 0
        outFile.write(reinterpret_cast<const _char*>(&iFlag), sizeof(_int));
    }

    // 파일 닫기
    outFile.close();

    MessageBox(0, L"Successfully saved navigation data!", L"Success", MB_OK);
}

void CNavigationTool::Load_NavigationData(const _string& strFilename)
{
    ifstream inFile(strFilename, ios::binary);
    if (!inFile.is_open())
    {
        MessageBox(0, L"Failed to open file for loading navigation data!", L"Error", MB_OK);
        return;
    }

    m_vecCells.clear();
    m_vecCellFlags.clear();
    m_vecPickedPoints.clear();
    m_iSelectedCellIndex = -1;

    while (!inFile.eof())
    {
        _float3 vPoint[3];
        _int iFlag = 0;

        // 셀과 플래그 읽어들이기
        inFile.read(reinterpret_cast<_char*>(vPoint), sizeof(_float3) * 3);
        inFile.read(reinterpret_cast<_char*>(&iFlag), sizeof(_int));

        if (inFile.eof())
            break;

        // 읽어들인 셀과 플래그 삽입
        m_vecCells.push_back({ vPoint[0], vPoint[1], vPoint[2] });
        m_vecCellFlags.push_back(iFlag);

        // 정점 중복 저장 없이 추가
        for (const auto& point : vPoint)
        {
            auto iter = find_if(m_vecPickedPoints.begin(), m_vecPickedPoints.end(),
                [&](const _float3& p)
                {
                    return m_pGameInstance->Compare_Float3(p, point);
                });

            if (iter == m_vecPickedPoints.end())
                m_vecPickedPoints.push_back(point);
        }
    }

    // 파일 닫기
    inFile.close();

    MessageBox(0, L"Navigation loaded successfully!", L"Success", MB_OK);
}

CNavigationTool* CNavigationTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNavigationTool* pInstance = new CNavigationTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CNavigationTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNavigationTool::Free()
{
	__super::Free();

    m_vecCells.clear();
    m_vecCellFlags.clear();
    m_vecPickedPoints.clear();
    m_vecNavFileList.clear();

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
    Safe_Release(m_pGameInstance);
}