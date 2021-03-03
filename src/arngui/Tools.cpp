#include "Tools.h"


int wxCALLBACK MyCompareFunction(wxIntPtr item1, wxIntPtr item2, long aaa)
{
	/*
   if (item1 < item2) return 1;
   if (item1 > item2) return -1;
   return 0;
   */

	SORTINFO * SortData = (SORTINFO *)aaa;
	wxListCtrl * pListCtrl = SortData->pListCtrl;
    wxListItem listItem1, listItem2;
	
	//use only col 0
	SortData->col = 0;

	long index1 = pListCtrl->FindItem(-1, item1); // get the index of the first item
	long index2 = pListCtrl->FindItem(-1, item2); // get the index of the second item

	listItem1.SetId(index1);
	listItem1.SetColumn(SortData->col);

	listItem2.SetId(index2);
	listItem2.SetColumn(SortData->col);

	listItem1.m_mask = listItem2.m_mask = wxLIST_MASK_TEXT;
	pListCtrl->GetItem(listItem1);// get the text of the first item
	pListCtrl->GetItem(listItem2);// get the text of the second item

	wxString w1 = listItem1.GetText();
	wxString w2 = listItem2.GetText();

	//now compare the strings
	int a = w2.Cmp(w1);

	//ascending or descending ?
    if (SortData->b_Sort) a = -a;

   //ouput to debug
   //printf("comparing in mode %d > %s and > %s result %d\n",SortData->b_Sort,static_cast<const char*>(w1),static_cast<const char*>(w2),a);
   
   return a;

}

void UpdateArrow(SORTINFO * SortData)
{
		wxListCtrl *pListCtrl = SortData->pListCtrl;
		//pListCtrl->SetImageList(m_imageListSmall, wxIMAGE_LIST_SMALL);
		wxListItem item;
		item.SetMask(wxLIST_MASK_IMAGE);

		if (SortData->b_Sort) item.SetImage(0);
		else item.SetImage(1);

		pListCtrl->SetColumn(0, item);
		item.SetImage(-1);
}