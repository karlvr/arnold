#include <wx/listctrl.h>


struct SORTINFO
{
	int col;
	wxListCtrl * pListCtrl;
	bool b_Sort;
};

int wxCALLBACK MyCompareFunction(wxIntPtr item1, wxIntPtr item2, long SortData);
void UpdateArrow(SORTINFO * SortData);