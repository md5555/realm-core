#ifndef __TDB_COLUMN_TABLE__
#define __TDB_COLUMN_TABLE__

#include "Column.h"

class Table;

class ColumnTable : public Column {
public:
	ColumnTable(size_t ref_specSet, Array* parent=NULL, size_t pndx=0, Allocator& alloc=GetDefaultAllocator());
	ColumnTable(size_t ref_column, size_t ref_specSet, Array* parent=NULL, size_t pndx=0, Allocator& alloc=GetDefaultAllocator());

	Table GetTable(size_t ndx);
	const Table GetTable(size_t ndx) const;
	Table* GetTablePtr(size_t ndx);
	size_t GetTableSize(size_t ndx) const;

	bool Add();
	void Insert(size_t ndx);
	void Delete(size_t ndx);
	void Clear(size_t ndx);

protected:
	
#ifdef _DEBUG
	virtual void LeafToDot(std::ostream& out, const Array& array) const;
#endif //_DEBUG
	
	size_t m_ref_specSet;
};

#endif //__TDB_COLUMN_TABLE__
