//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkToolsTree.cpp
// description : Tools for tree view
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------
#include "gtkToolsTree.h"
#include <string>

using namespace std;
//----------------------------------------------------------------------------
// GtkTools : TreeView
//----------------------------------------------------------------------------
namespace GtkTools {
// Add a column with one text renderer. Name of column is utf8Name
GtkCellRenderer *
gtkTreeAppendColumnText(GtkTreeView *treeView,
                        const int &dataModelIndex,
                        const std::string &utf8Name,
                        const int &sortModelIndex/*=-1*/)

{
  GtkCellRenderer * renderer;
  GtkTreeViewColumn * column;

  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title(column, utf8Name.c_str());

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column,renderer,TRUE);
  gtk_tree_view_column_set_attributes (column, renderer,"text", dataModelIndex,NULL);
  if (sortModelIndex>=0)
    gtk_tree_view_column_set_sort_column_id (column,sortModelIndex);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeView),column);

  return renderer;
}

// Get text in column id
bool
gtkTreeGetSelectedText(GtkTreeView *treeView,
                       const int &dataModelIndex,
                       std::string &valueOut)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (treeView);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gchar *txt;
    gtk_tree_model_get (model, &iter, dataModelIndex, &txt, -1);
    if (txt)
    {
      valueOut=txt;
      g_free(txt);
      return true;
    }
  }
  return false;
}

// Get uint in column id
bool
gtkTreeGetSelectedUInt(GtkTreeView *treeView,
                       const int &dataModelIndex,
                       unsigned int&valueOut)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (treeView);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
  {
    gtk_tree_model_get (model, &iter, dataModelIndex, &valueOut, -1);
    return true;
  }
  return false;
}

//--------------------- Private fonctions ------------------------------------
static void
_findExpandedPaths(GtkTreeView *treeView,
                   GtkTreeModel *tree_model,
                   GtkTreeIter *iter,
                   vector<GtkTreePath*> &list);


//------------------------------ Paths ---------------------------------------
// Backup Expandeds Paths Of a Tree
void
findExpandedPaths(GtkTreeView *treeView,
                  GtkTreeIter *iter,
                  vector<GtkTreePath*> &list)
{
  GtkTreeModel *model=gtk_tree_view_get_model(treeView);
  if (model)
    _findExpandedPaths(treeView, model, iter,list);
}

static void
_findExpandedPaths(GtkTreeView *treeView,
                   GtkTreeModel *tree_model,
                   GtkTreeIter *iter,
                   vector<GtkTreePath*> &list)
{
  GtkTreeIter iterChild;

  if (gtk_tree_model_iter_children(GTK_TREE_MODEL(tree_model),&iterChild,iter))
  {
  // On regarde les childs
    _findExpandedPaths(treeView,tree_model,&iterChild,list);

    if (iter)
    {
    // on regarde s'il est étendu
      GtkTreePath *path=gtk_tree_model_get_path(GTK_TREE_MODEL(tree_model),iter);
      if (path)
      {
        if (gtk_tree_view_row_expanded(treeView,path))
          list.push_back(path);
        else
          gtk_tree_path_free(path);
      }
    }
  }
  if (iter)
    while (gtk_tree_model_iter_next(GTK_TREE_MODEL(tree_model),iter))
    {
    // On regarde les childs
      _findExpandedPaths(treeView,tree_model,iter,list);
    }
}

// Restore Expandeds Paths Of a Tree
void
restoreExpandedPaths(GtkTreeView *treeView,vector<GtkTreePath*> &list)
{
  for (int cpt=list.size()-1;cpt>=0;cpt--)
  {
    gtk_tree_view_expand_to_path(treeView,list[cpt]);
    gtk_tree_path_free(list[cpt]);
    list[cpt]=NULL;
  }
  list.clear();
}

} // end of namespace GtkTools
//---End-Of-File---------------------------------------------------------------
