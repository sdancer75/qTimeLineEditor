// ChangeDurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ZTimeAlloc.h"
#include "ChangeDurationDlg.h"


// CChangeDurationDlg dialog

IMPLEMENT_DYNAMIC(CChangeDurationDlg, CDialog)

CChangeDurationDlg::CChangeDurationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeDurationDlg::IDD, pParent)
	, m_nSliceDuration(0)
	, m_strDuration(_T(""))
{

}

CChangeDurationDlg::~CChangeDurationDlg()
{
}

void CChangeDurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DURATION, m_strDuration);
}


BEGIN_MESSAGE_MAP(CChangeDurationDlg, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_DURATION, &CChangeDurationDlg::OnDeltaposSpinDuration)
END_MESSAGE_MAP()


// CChangeDurationDlg message handlers

BOOL CChangeDurationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_strDuration.Format("%d",m_nSliceDuration);
	UpdateData(FALSE);

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeDurationDlg::OnDeltaposSpinDuration(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
