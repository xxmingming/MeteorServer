#include "stdafx.h"
#include "KcpServer.h"
int udp_output(const char *buf, int len, ikcpcb *kcp, void *user);
CUserInfo::CUserInfo()
{
	m_bEmpty					= true;
	m_bDirty					= true;
	m_pxPlayerObject			= NULL;
	m_pGateInfo					= NULL;
	m_pRoom = NULL;
	m_nCertification = 0;
}

bool CUserInfo::IsEmpty()
{
	return m_bEmpty;
}

void CUserInfo::SetName(const char * pszName)
{
	int namelen = min(strlen(pszName), 18);
	memcpy(m_szCharName, pszName, namelen);
	for (int i = namelen; i < 20; i++)
		m_szCharName[i] = 0x00;
}

void CUserInfo::CloseUserHuman()
{
	Lock();
	if (m_pxPlayerObject)
	{
		m_pxPlayerObject->Lock();
		m_pxPlayerObject->m_pUserInfo = NULL;
		m_pxPlayerObject->m_bEmpty = TRUE;
		m_pxPlayerObject->Unlock();
		m_pxPlayerObject = NULL;
	}
	m_bEmpty = TRUE;
	Unlock();
}

void CUserInfo::DoClientCertification(UINT32 clientV, std::string name)
{
	m_nCertification = 1;
	m_nClientVersion = clientV;
	SetName(name.c_str());
}

void CUserInfo::KcpRelease()
{
	if (m_pKcp != NULL)
	{
		ikcp_release(m_pKcp);
		m_pKcp = NULL;
		m_nKcpReveivedBytes = 0;
	}
}

void CUserInfo::KcpUpdate(int mill)
{
	if (m_pKcp != NULL)
		ikcp_update(m_pKcp, mill);
}

void CUserInfo::InitKcp(KcpServer * pSvr)
{
	m_pKcpSvr = pSvr;
	m_pKcp = ikcp_create(this->m_nUserServerIndex, (void*)this);
	m_pKcp->output = udp_output;
	ikcp_wndsize(m_pKcp, 128, 128);
	ikcp_nodelay(m_pKcp, 1, 10, 2, 1);
}

int	CUserInfo::KcpInput(char * Buffer, int nRecvBytes)
{
	return ikcp_input(m_pKcp, Buffer, nRecvBytes);
}

void CUserInfo::OnReceivedMsg()
{
	m_nremoteaddrlen = m_pKcpSvr->fromlen;
	memmove(&m_remoteaddr, &m_pKcpSvr->from, m_nremoteaddrlen);
	int received = ikcp_recv(m_pKcp, (char*)(&m_pKcpBuffer[m_nKcpReveivedBytes]), KCP_PACKET_SIZE - m_nKcpReveivedBytes);
	m_nKcpReveivedBytes += received;
	//尝试解出kcp里的消息包
	//4字节长度-4字节消息ID
	ExtractPacket();
}

void CUserInfo::ExtractPacket()
{
	int offset = 0;
	while (m_nKcpReveivedBytes > 8)
	{
		CMsg * pMsg = (CMsg*)(&m_pKcpBuffer[offset]);

		//整个包大小，小于当前已存在数据大小
		if (pMsg->Size < m_nKcpReveivedBytes)
		{
			char * pData = ((char*)pMsg) + sizeof(CMsg);
			//处理包内容
			switch (pMsg->Message)
			{
				//初始化。得到对端UDP
				case MeteorMsg_MsgType_SyncCommand:
				break;
				//角色
				case MeteorMsg_Command_SpawnPlayer:
					OnEnterLevel(pData, pMsg->Size);
					m_pKcpSvr->OnPlayerSpawn(this, pData, pMsg->Size);
				break;
				default:printf("recv kcp message"); break;
			}

			m_nKcpReveivedBytes -= pMsg->Size;
			offset += pMsg->Size;
		}
	}

	//把尾部内存拷贝到前面来.
	if (m_nKcpReveivedBytes != 0 && offset != 0)
		memmove(&m_pKcpBuffer, &m_pKcpBuffer[offset], m_nKcpReveivedBytes);
}

void CUserInfo::OnEnterLevel(char * pData, int size)
{
	PlayerEventData evt;
	evt.ParseFromArray(pData, size);
	int nIndex = g_xPlayerObjectArr.GetFreeKey();
	if (nIndex >= 0)
	{
		m_pxPlayerObject = &g_xPlayerObjectArr[nIndex];
		m_pxPlayerObject->Lock();
		m_pxPlayerObject->Reset(pUser);
		m_pxPlayerObject->m_bEmpty = false;
		m_pxPlayerObject->m_nArrIndex = nIndex;
		m_pxPlayerObject->SetCharName(pUser->m_szCharName);
		m_pxPlayerObject->m_Ability.Camp = evt.camp();
		m_pxPlayerObject->m_Ability.Model = evt.model();
		m_pxPlayerObject->m_Ability.Weapon = evt.weapon();
		m_pxPlayerObject->Unlock();
	}
	else
	{
		//print("nIndex < 0");
	}
}

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	CUserInfo * pUser = (CUserInfo*)user;
	sendto(pUser->m_pKcpSvr->sock, buf, len, 0, (SOCKADDR*)&pUser->m_remoteaddr, pUser->m_nremoteaddrlen);
	return 0;
}