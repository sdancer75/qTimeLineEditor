#pragma once


// CChangeDurationDlg dialog

class CChangeDurationDlg : public CDialog
{
	DECLARE_DYNAMIC(CChangeDurationDlg)

public:
	CChangeDurationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChangeDurationDlg();

// Dialog Data
	enum { IDD = IDD_CHANGEDURATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nSliceDuration;
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaposSpinDuration(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_strDuration;
};
