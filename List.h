#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

template <typename T>
class List{
private:
	struct Node{
		Node *nNext, *nPrev;
		T Data;
	};
	Node *nHead, *nTail;
	int iItemCount;
public:
	List();
	~List();
	void Add(T Data);
	void Remove(int iPos);
	void Fill(int iItems, T Data);
	T Get(int iPos);
	void Modify(int iPos, T Data);
	int Count() { return iItemCount; }
	void Clear();
};

#endif