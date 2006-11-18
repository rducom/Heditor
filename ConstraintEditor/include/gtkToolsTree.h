//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkToolsTree.h
// description : Tools for treeviews
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------
#ifndef GTKTOOLS_TREE_H
#define GTKTOOLS_TREE_H 1

#include <gtk/gtk.h>
#include <vector>

//----------------------------------------------------------------------------
// GtkTools : TreeView
//----------------------------------------------------------------------------
namespace GtkTools {

//---------------------------- TreeModel -------------------------------------
//! Add a column with one text renderer. Name of column is utf8Name
//! sortModelIndex is used to indicate witch column id is used to sort this
//! column. If sortModelIndex<0 => no sort!
GtkCellRenderer *
gtkTreeAppendColumnText(GtkTreeView *treeView,
                        const int &dataModelIndex,
                        const std::string &utf8Name,
                        const int &sortModelIndex=-1);

//! Get text in column id
bool
gtkTreeGetSelectedText(GtkTreeView *treeView,
                       const int &dataModelIndex,
                       std::string &valueOut);

//! Get uint in column id
bool
gtkTreeGetSelectedUInt(GtkTreeView *treeView,
                       const int &dataModelIndex,
                       unsigned int&valueOut);


//------------------------------ Paths ---------------------------------------
//! Backup Expandeds Paths Of a Tree
void
findExpandedPaths(GtkTreeView *treeView,
                  GtkTreeIter *iter,
                  std::vector<GtkTreePath*> &listOut);

//! Restore Expandeds Paths Of a Tree
//! list will be empty next call to restoreExpandedPaths
void
restoreExpandedPaths(GtkTreeView *treeView,
                     std::vector<GtkTreePath*> &list);
} // end of namespace GtkTools
#endif // end of GTKTOOLS_TREE_H
//---End-Of-File---------------------------------------------------------------
