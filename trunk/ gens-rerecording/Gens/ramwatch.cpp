#include "resource.h"
#include "gens.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "misc.h"
#include "mem_z80.h"
#include "vdp_io.h"
#include "save.h"
#include "ram_search.h"
#include "g_main.h"
#include <assert.h>
#include <windows.h>
#include <commctrl.h>

static HMENU ramwatchmenu;
static HMENU rwrecentmenu;
char *rw_recent_files[] = { 0 ,0 ,0 ,0 ,0 };
const unsigned int RW_MENU_FIRST_RECENT_FILE = 600;
const unsigned int RW_MAX_NUMBER_OF_RECENT_FILES = sizeof(rw_recent_files)/sizeof(*rw_recent_files);
bool RWfileChanged = false; //Keeps track of whether the current watch file has been changed, if so, ramwatch will prompt to save changes
bool AutoRWLoad = false;    //Keeps track of whether Auto-load is checked

void UpdateRW_RMenu(HMENU menu, char **strs, unsigned int mitem, unsigned int baseid)
{
	MENUITEMINFO moo;
	int x;

	moo.cbSize = sizeof(moo);
	moo.fMask = MIIM_SUBMENU | MIIM_STATE;

	GetMenuItemInfo(GetSubMenu(ramwatchmenu, 0), mitem, FALSE, &moo);
	moo.hSubMenu = menu;
	moo.fState = strs[0] ? MFS_ENABLED : MFS_GRAYED;

	SetMenuItemInfo(GetSubMenu(ramwatchmenu, 0), mitem, FALSE, &moo);

	// Remove all recent files submenus
	for(x = 0; x < RW_MAX_NUMBER_OF_RECENT_FILES; x++)
	{
		RemoveMenu(menu, baseid + x, MF_BYCOMMAND);
	}

	// Recreate the menus
	for(x = RW_MAX_NUMBER_OF_RECENT_FILES - 1; x >= 0; x--)
	{  
		char tmp[128 + 5];

		// Skip empty strings
		if(!strs[x])
		{
			continue;
		}

		moo.cbSize = sizeof(moo);
		moo.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE;

		// Fill in the menu text.
		if(strlen(strs[x]) < 128)
		{
			sprintf(tmp, "&%d. %s", ( x + 1 ) % 10, strs[x]);
		}
		else
		{
			sprintf(tmp, "&%d. %s", ( x + 1 ) % 10, strs[x] + strlen( strs[x] ) - 127);
		}

		// Insert the menu item
		moo.cch = strlen(tmp);
		moo.fType = 0;
		moo.wID = baseid + x;
		moo.dwTypeData = tmp;
		InsertMenuItem(menu, 0, 1, &moo);
	}
}

void UpdateRWRecentArray(const char* addString, char** bufferArray, unsigned int arrayLen, HMENU menu, unsigned int menuItem, unsigned int baseId)
{
	// Try to find out if the filename is already in the recent files list.
	for(unsigned int x = 0; x < arrayLen; x++)
	{
		if(bufferArray[x])
		{
			if(!strcmp(bufferArray[x], addString))    // Item is already in list.
			{
				// If the filename is in the file list don't add it again.
				// Move it up in the list instead.

				int y;
				char *tmp;

				// Save pointer.
				tmp = bufferArray[x];
				
				for(y = x; y; y--)
				{
					// Move items down.
					bufferArray[y] = bufferArray[y - 1];
				}

				// Put item on top.
				bufferArray[0] = tmp;

				// Update the recent files menu
				UpdateRW_RMenu(menu, bufferArray, menuItem, baseId);

				return;
			}
		}
	}

	// The filename wasn't found in the list. That means we need to add it.

	// If there's no space left in the recent files list, get rid of the last
	// item in the list.
	if(bufferArray[arrayLen - 1])
	{
		free(bufferArray[arrayLen - 1]);
	}

	// Move the other items down.
	for(unsigned int x = arrayLen - 1; x; x--)
	{
		bufferArray[x] = bufferArray[x - 1];
	}

	// Add the new item.
	bufferArray[0] = (char*)malloc(strlen(addString) + 1); //mbg merge 7/17/06 added cast
	strcpy(bufferArray[0], addString);

	// Update the recent files menu
	UpdateRW_RMenu(menu, bufferArray, menuItem, baseId);
}

/**
* Add a filename to the recent files list.
*
* @param filename Name of the file to add.
**/
void RWAddRecentFile(const char *filename)
{
	UpdateRWRecentArray(filename, rw_recent_files, RW_MAX_NUMBER_OF_RECENT_FILES, rwrecentmenu, RAMMENU_FILE_RECENT, RW_MENU_FIRST_RECENT_FILE);
}

void OpenRWRecentFile(int memwRFileNumber)
{
	int rnum=memwRFileNumber;
		if (rnum > RW_MAX_NUMBER_OF_RECENT_FILES) return; //just in case
		
	char* x = rw_recent_files[rnum];
	if (x == NULL) return; //If no recent files exist just return.  Useful for Load last file on startup (or if something goes screwy)
	//char watchfcontents[2048];
	
	if (rnum != 0) //Change order of recent files if not most recent
		RWAddRecentFile(x);
	//loadwatches here

	
RWfileChanged = false;
}

bool Save_Watches()
{
	strncpy(Str_Tmp,Rom_Name,512);
	strcat(Str_Tmp,".wch");
	if(Change_File_S(Str_Tmp, Gens_Path, "Save Watches", "GENs Watchlist\0*.wch\0All Files\0*.*\0\0", "wch"))
	{
		FILE *WatchFile = fopen(Str_Tmp,"r+b");
		if (!WatchFile) WatchFile = fopen(Str_Tmp,"w+b");
		fputc(SegaCD_Started?'1':(_32X_Started?'2':'0'),WatchFile);
		fputc('\n',WatchFile);
		sprintf(Str_Tmp,"%d\n",WatchCount);
		fputs(Str_Tmp,WatchFile);
		const char DELIM = '\t';
		for (int i = 0; i < WatchCount; i++)
		{
			sprintf(Str_Tmp,"%05X%c%08X%c%c%c%c%c%d%c%s\n",rswatches[i].Index,DELIM,rswatches[i].Address,DELIM,rswatches[i].Size,DELIM,rswatches[i].Type,DELIM,rswatches[i].WrongEndian,DELIM,rsaddrs[rswatches[i].Index].comment);
			fputs(Str_Tmp,WatchFile);
		}
		fclose(WatchFile);
		return true;
	}
	return false;
}

bool Load_Watches()
{
	strncpy(Str_Tmp,Rom_Name,512);
	strcat(Str_Tmp,".wch");
	const char DELIM = '\t';
	if(Change_File_L(Str_Tmp, Watch_Dir, "Load Watches", "GENs Watchlist\0*.wch\0All Files\0*.*\0\0", "wch"))
	{
		FILE *WatchFile = fopen(Str_Tmp,"rb");
		if (!WatchFile)
		{
			MessageBox(NULL,"Error opening file.","ERROR",MB_OK);
			return false;
		}
		AddressWatcher Temp;
		char mode;
		fgets(Str_Tmp,1024,WatchFile);
		sscanf(Str_Tmp,"%c%*s",&mode);
		if ((mode == '1' && !(SegaCD_Started)) || (mode == '2' && !(_32X_Started)))
		{
			char Device[8];
			strcpy(Device,(mode > '1')?"32X":"SegaCD");
			sprintf(Str_Tmp,"Warning: %s not started. \nWatches for %s addresses will be ignored.",Device,Device);
			MessageBox(NULL,Str_Tmp,"Device Mismatch",MB_OK);
		}
		int WatchAdd;
		fgets(Str_Tmp,1024,WatchFile);
		sscanf(Str_Tmp,"%d%*s",&WatchAdd);
		WatchAdd+=WatchCount;
		for (int i = WatchCount; i < WatchAdd; i++)
		{
			while (i < 0)
				i++;
			do {
				fgets(Str_Tmp,1024,WatchFile);
			} while (Str_Tmp[0] == '\n');
			sscanf(Str_Tmp,"%05X%*c%08X%*c%c%*c%c%*c%d",&(Temp.Index),&(Temp.Address),&(Temp.Size),&(Temp.Type),&(Temp.WrongEndian));
			Temp.WrongEndian = 0;
			if (SegaCD_Started && (mode != '1'))
				Temp.Index += SEGACD_RAM_SIZE;
			if ((mode == '1') && !SegaCD_Started)
				Temp.Index -= SEGACD_RAM_SIZE;
			if ((unsigned int)Temp.Index > (unsigned int)CUR_RAM_MAX)
			{
				i--;
				WatchAdd--;
			}
			else
			{
				char *Comment = strrchr(Str_Tmp,DELIM) + 1;
				*strrchr(Comment,'\n') = '\0';
				InsertWatch(Temp,Comment);
			}
		}
		fclose(WatchFile);
		if (RamWatchHWnd)
			ListView_SetItemCount(GetDlgItem(RamWatchHWnd,IDC_WATCHLIST),WatchCount);
		return true;
	}
	return false;
}

void ResetWatches()
{
	for (;WatchCount>=0;WatchCount--)
	{
		rsaddrs[rswatches[WatchCount].Index].flags &= ~RS_FLAG_WATCHED;
		free(rsaddrs[rswatches[WatchCount].Index].comment);
		rsaddrs[rswatches[WatchCount].Index].comment = NULL;
	}
	WatchCount++;
	if (RamWatchHWnd)
		ListView_SetItemCount(GetDlgItem(RamWatchHWnd,IDC_WATCHLIST),WatchCount);
}

void RemoveWatch(int watchIndex)
{
	free(rsaddrs[rswatches[watchIndex].Index].comment);
	rsaddrs[rswatches[watchIndex].Index].comment = NULL;
	for (int i = watchIndex; i <= WatchCount; i++)
		rswatches[i] = rswatches[i+1];
	rsaddrs[rswatches[WatchCount].Index].flags &= ~RS_FLAG_WATCHED;
	WatchCount--;
}

LRESULT CALLBACK EditWatchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) //Gets info for a RAM Watch, and then inserts it into the Watch List
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;
	static int index;
	static char s,t = s = 0;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			//SetWindowPos(hDlg, NULL, max(0, r.left + (dx1 - dx2)), max(0, r.top + (dy1 - dy2)), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			index = (int)lParam;
			sprintf(Str_Tmp,"%08X",rswatches[index].Address);
			SetDlgItemText(hDlg,IDC_EDIT_COMPAREADDRESS,Str_Tmp);
			if (rsaddrs[rswatches[index].Index].comment != NULL)
				SetDlgItemText(hDlg,IDC_PROMPT_EDIT,rsaddrs[rswatches[index].Index].comment);
			s = rswatches[index].Size;
			t = rswatches[index].Type;
			switch (s)
			{
				case 'b':
					SendDlgItemMessage(hDlg, IDC_1_BYTE, BM_SETCHECK, BST_CHECKED, 0);
					break;
				case 'w':
					SendDlgItemMessage(hDlg, IDC_2_BYTES, BM_SETCHECK, BST_CHECKED, 0);
					break;
				case 'd':
					SendDlgItemMessage(hDlg, IDC_4_BYTES, BM_SETCHECK, BST_CHECKED, 0);
					break;
				default:
					s = 0;
					break;
			}
			switch (t)
			{
				case 's':
					SendDlgItemMessage(hDlg, IDC_SIGNED, BM_SETCHECK, BST_CHECKED, 0);
					break;
				case 'u':
					SendDlgItemMessage(hDlg, IDC_UNSIGNED, BM_SETCHECK, BST_CHECKED, 0);
					break;
				case 'h':
					SendDlgItemMessage(hDlg, IDC_HEX, BM_SETCHECK, BST_CHECKED, 0);
					break;
				default:
					t = 0;
					break;
			}
			return true;
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_SIGNED:
					t='s';
					return true;
				case IDC_UNSIGNED:
					t='u';
					return true;
				case IDC_HEX:
					t='h';
					return true;
				case IDC_1_BYTE:
					s = 'b';
					return true;
				case IDC_2_BYTES:
					s = 'w';
					return true;
				case IDC_4_BYTES:
					s = 'd';
					return true;
				case IDOK:
				{
					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}
					if (s && t)
					{
						AddressWatcher Temp;
						Temp.Size = s;
						Temp.Type = t;
						Temp.WrongEndian = false; //replace this when I get little endian working properly
						GetDlgItemText(hDlg,IDC_EDIT_COMPAREADDRESS,Str_Tmp,1024);
						char *addrstr = Str_Tmp;
						if (strlen(Str_Tmp) > 8) addrstr = &(Str_Tmp[strlen(Str_Tmp) - 9]);
						sscanf(addrstr,"%08X",&(Temp.Address));
						Temp.Index = getSearchIndex(Temp.Address,0,CUR_RAM_MAX);
						if (Temp.Index > -1)
						{
							GetDlgItemText(hDlg,IDC_PROMPT_EDIT,Str_Tmp,80);
							if (index < WatchCount) RemoveWatch(index);
							InsertWatch(Temp,Str_Tmp);
							if(RamWatchHWnd)
							{
								ListView_SetItemCount(GetDlgItem(RamWatchHWnd,IDC_WATCHLIST),WatchCount);
							}
							DialogsOpen--;
							EndDialog(hDlg, true);
						}
						else 
							MessageBox(NULL,"Error: Invalid Address.","ERROR",MB_OK);
					}
					else
					{
						strcpy(Str_Tmp,"Error:");
						if (!s)
							strcat(Str_Tmp," Size must be specified.");
						if (!t)
							strcat(Str_Tmp," Type must be specified.");
						MessageBox(NULL,Str_Tmp,"ERROR",MB_OK);
					}
					return true;
					break;
				}
				case ID_CANCEL:
					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}
					DialogsOpen--;
					EndDialog(hDlg, false);
					return false;
					break;
			}
			break;

		case WM_CLOSE:
			if (Full_Screen)
			{
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}
			DialogsOpen--;
			EndDialog(hDlg, false);
			return false;
			break;
	}

	return false;
}
LRESULT CALLBACK RamWatchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;
	static int watchIndex=0;

	switch(uMsg)
	{
		case WM_INITDIALOG: {
			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			ramwatchmenu=GetMenu(hDlg);
			// push it away from the main window if we can
			const int width = (r.right-r.left); 
			const int width2 = (r2.right-r2.left); 
			if(r.left+width2 + width < GetSystemMetrics(SM_CXSCREEN))
			{
				r.right += width;
				r.left += width;
			}
			else if((int)r.left - (int)width2 > 0)
			{
				r.right -= width2;
				r.left -= width2;
			}

			SetWindowPos(hDlg, NULL, r.left, r.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			char names[3][11] = {"Address","Value","Notes"};
			int widths[3] = {62,64,64+51+53};
			init_list_box(GetDlgItem(hDlg,IDC_WATCHLIST),names,3,widths);
			if (!ResultCount)
				reset_address_info();
			else
				signal_new_frame();
			ListView_SetItemCount(GetDlgItem(hDlg,IDC_WATCHLIST),WatchCount);
			if (!noMisalign) SendDlgItemMessage(hDlg, IDC_MISALIGN, BM_SETCHECK, BST_CHECKED, 0);
			if (littleEndian) SendDlgItemMessage(hDlg, IDC_ENDIAN, BM_SETCHECK, BST_CHECKED, 0);
			return true;
			break;
		}
		case WM_INITMENU:
		CheckMenuItem(ramwatchmenu, RAMMENU_FILE_AUTOLOAD, AutoRWLoad ? MF_CHECKED : MF_UNCHECKED);
		break;

		case WM_NOTIFY:
		{
			LPNMHDR lP = (LPNMHDR) lParam;
			switch (lP->code)
			{
				case LVN_GETDISPINFO:
				{
					LV_DISPINFO *Item = (LV_DISPINFO *)lParam;
					Item->item.mask = LVIF_TEXT;
					Item->item.state = 0;
					Item->item.iImage = 0;
					const unsigned int iNum = Item->item.iItem;
					static char num[11];
					switch (Item->item.iSubItem)
					{
						case 0:
							sprintf(num,"%08X",rswatches[iNum].Address);
							Item->item.pszText = num;
							return true;
						case 1: {
							int i;
							int size;
							switch (rswatches[iNum].Size)
							{

								case 'w':
									size = 2;
									break;
								case 'd':
									size = 4;
									break;
								case 'b':
								default:
									size = 1;
									break;									
							}
							i = sizeConv(rswatches[Item->item.iItem].Index,rswatches[Item->item.iItem].Size);
//							for (int j = 1; j < size; j++)
//								i |= rsaddrs[rswatches[Item->item.iItem].Index + j].cur << (8 * j);
//							Byte_Swap(&i,size);
							sprintf(num,
								((rswatches[iNum].Type == 's')?
									"%d":
									(rswatches[iNum].Type == 'u')?
										"%u":
										"%X"),
								(rswatches[iNum].Type != 's')?
									i:
									((rswatches[iNum].Size == 'b')?
										(char)(i&0xff):
										(rswatches[iNum].Size == 'w')?
											(short)(i&0xffff):
											(long)(i&0xffffffff)));
							Item->item.pszText = num;
						}	return true;
						case 2:
							Item->item.pszText = rsaddrs[rswatches[iNum].Index].comment ? rsaddrs[rswatches[iNum].Index].comment : "";
							return true;

						default:
							return false;
					}
				}
				//case LVN_ODCACHEHINT: //Copied this bit from the MSDN virtual listbox code sample. Eventually it should probably do something.
				//{
				//	LPNMLVCACHEHINT   lpCacheHint = (LPNMLVCACHEHINT)lParam;
				//	return 0;
				//}
				//case LVN_ODFINDITEM: //Copied this bit from the MSDN virtual listbox code sample. Eventually it should probably do something.
				//{	
				//	LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)lParam;
				//	return 0;
				//}
			}
		}

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ACCEL_CTRL_S:
				case RAMMENU_FILE_SAVE:
					//QuickSaveWatches();
					break;

				case ACCEL_CTRL_SHIFT_S:
				case RAMMENU_FILE_SAVEAS:	
				//case IDC_C_SAVE:
					return Save_Watches();
				case ACCEL_CTRL_O:
				case RAMMENU_FILE_OPEN:
				//case IDC_C_LOAD:
					return Load_Watches();
				case ACCEL_CTRL_N:
				case RAMMENU_FILE_NEW:
				//case IDC_C_RESET:
					ResetWatches();
					return true;
				case RAMMENU_WATCHES_REMOVEWATCH:
				case IDC_C_SEARCH:
					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST));
					RemoveWatch(watchIndex);
					ListView_SetItemCount(GetDlgItem(hDlg,IDC_WATCHLIST),WatchCount);					
					return true;
				case RAMMENU_WATCHES_EDITWATCH:
				case IDC_C_WATCH_EDIT:
					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST));
					DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITWATCH), hDlg, (DLGPROC) EditWatchProc,(LPARAM) watchIndex);
					return true;
				case RAMMENU_WATCHES_NEWWATCH:
				case IDC_C_WATCH:
					rswatches[WatchCount].Address = rswatches[WatchCount].Index = rswatches[WatchCount].WrongEndian = 0;
					rswatches[WatchCount].Size = 'b';
					rswatches[WatchCount].Type = 's';
					DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITWATCH), hDlg, (DLGPROC) EditWatchProc,(LPARAM) WatchCount);
					return true;
				case RAMMENU_WATCHES_DUPLICATEWATCH:
				case IDC_C_WATCH2:
					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST));
					rswatches[WatchCount].Address = rswatches[watchIndex].Address;
					rswatches[WatchCount].Index = rswatches[watchIndex].Index;
					rswatches[WatchCount].WrongEndian = rswatches[watchIndex].WrongEndian;
					rswatches[WatchCount].Size = rswatches[watchIndex].Size;
					rswatches[WatchCount].Type = rswatches[watchIndex].Type;
					DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITWATCH), hDlg, (DLGPROC) EditWatchProc,(LPARAM) WatchCount);
					return true;
				case RAMMENU_WATCHES_MOVEUP:
				case IDC_C_WATCH_UP:
				{
					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST));
					if (watchIndex == 0)
						return true;
					void *tmp = malloc(sizeof(AddressWatcher));
					memcpy(tmp,&(rswatches[watchIndex]),sizeof(AddressWatcher));
					memcpy(&(rswatches[watchIndex]),&(rswatches[watchIndex - 1]),sizeof(AddressWatcher));
					memcpy(&(rswatches[watchIndex - 1]),tmp,sizeof(AddressWatcher));
					free(tmp);
					ListView_SetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST),watchIndex-1);
					ListView_SetItemState(GetDlgItem(hDlg,IDC_WATCHLIST),watchIndex-1,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
					ListView_SetItemCount(GetDlgItem(hDlg,IDC_WATCHLIST),WatchCount);
					return true;
				}
				case RAMMENU_WATCHES_MOVEDOWN:
				case IDC_C_WATCH_DOWN:
				{
					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST));
					if (watchIndex >= WatchCount - 1)
						return true;
					void *tmp = malloc(sizeof(AddressWatcher));
					memcpy(tmp,&(rswatches[watchIndex]),sizeof(AddressWatcher));
					memcpy(&(rswatches[watchIndex]),&(rswatches[watchIndex + 1]),sizeof(AddressWatcher));
					memcpy(&(rswatches[watchIndex + 1]),tmp,sizeof(AddressWatcher));
					free(tmp);
					ListView_SetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST),watchIndex+1);
					ListView_SetItemState(GetDlgItem(hDlg,IDC_WATCHLIST),watchIndex+1,LVIS_FOCUSED|LVIS_SELECTED,LVIS_FOCUSED|LVIS_SELECTED);
					ListView_SetItemCount(GetDlgItem(hDlg,IDC_WATCHLIST),WatchCount);
					return true;
				}
				case RAMMENU_FILE_AUTOLOAD:
				{
					AutoRWLoad ^= 1;
					CheckMenuItem(ramwatchmenu, RAMMENU_FILE_AUTOLOAD, AutoRWLoad ? MF_CHECKED : MF_UNCHECKED);
					break;
				}
				case IDC_C_ADDCHEAT:
				{
//					watchIndex = ListView_GetSelectionMark(GetDlgItem(hDlg,IDC_WATCHLIST)) | (1 << 24);
//					DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_EDITCHEAT), hDlg, (DLGPROC) EditCheatProc,(LPARAM) searchIndex);
				}
				case IDOK:
				case ACCEL_CTRL_W:
				case RAMMENU_FILE_CLOSE:
				//case IDCANCEL:
					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}
					DialogsOpen--;
					RamWatchHWnd = NULL;
					EndDialog(hDlg, true);
					return true;
			}
			break;

		case WM_CLOSE:
			if (Full_Screen)
			{
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}
			DialogsOpen--;
			RamWatchHWnd = NULL;
			EndDialog(hDlg, true);
			return true;
	}

	return false;
}
