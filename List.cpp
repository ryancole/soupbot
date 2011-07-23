#include "List.h"
#include <winsock2.h>
#include <windows.h>
#include <string.h>

template List<char *>;
template List<unsigned long>;
template List<SOCKET>;
template List<int>;

template <typename T>
List<T>::List()
{
	nHead = new Node;
	nHead->Data = NULL;
	nTail = nHead;
	iItemCount = 0;
}

template <typename T>
List<T>::~List()
{
	Clear();
	delete nHead;
	delete nTail;
}

template <typename T>
void List<T>::Add(T Data)
{
	Node *nTmp = nTail;
	nTail->nNext = new Node;
	nTail->nNext->Data = Data;
	nTail = nTail->nNext;
	nTail->nPrev = nTmp;
	nTail->nNext = NULL;
	iItemCount++;
}

template <typename T>
void List<T>::Remove(int iPos)
{
	int iPos2 = 0;
	Node *nTmp = nHead;
	while(nTmp != NULL){
		if(iPos2 == (iPos + 1))
			break;
		nTmp = nTmp->nNext;
		iPos2++;
	}
	if(nTmp == NULL)
		return;
	if(nTmp == nHead){
		nHead = nHead->nNext;
		delete nTmp;
		iItemCount--;
		return;
	}
	if(nTmp == nTail){
		nTail = nTail->nPrev;
		nTail->nNext = NULL;
		delete nTmp;
		iItemCount--;
		return;
	}
	nTmp->nPrev->nNext = nTmp->nNext;
	nTmp->nNext->nPrev = nTmp->nPrev;
	delete nTmp;
	iItemCount--;
}

template <typename T>
void List<T>::Fill(int iItems, T Data)
{
	Clear();
	for(int iCnt = 0; iCnt < iItems; iCnt++)
		Add(Data);
}

template <typename T>
T List<T>::Get(int iPos)
{
	Node *nTmp = nHead;
	int iPos2 = 0;
	while(nTmp != NULL){
		if(iPos2 == (iPos + 1))
			return nTmp->Data;
		nTmp = nTmp->nNext;
		iPos2++;
	}
	return NULL;
}

template <typename T>
void List<T>::Modify(int iPos, T Data)
{
	Node *nTmp = nHead;
	int iPos2 = 0;
	while(nTmp != NULL){
		if(iPos2 == (iPos + 1)){
			nTmp->Data = Data;
			return;
		}
		nTmp = nTmp->nNext;
		iPos2++;
	}
}

template <typename T>
void List<T>::Clear()
{
	Node *nTmp = nHead;
	while(nTmp != nTail){
		Node *nTmp2 = nTmp;
		nTmp = nTmp->nNext;
		delete nTmp2;
	}
	delete nTmp;
	nHead = new Node;
	nHead->Data = NULL;
	nTail = nHead;
	iItemCount = 0;
}