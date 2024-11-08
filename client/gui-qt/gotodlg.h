/***********************************************************************
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/

#ifndef FC__GOTODLG_H
#define FC__GOTODLG_H

#ifdef HAVE_CONFIG_H
#include <fc_config.h>
#endif

extern "C" {
#include "gotodlg_g.h"
}

// Qt
#include <QDialog>

// common
#include "dialogs.h"
#include "unit.h"

class QCheckBox;
class QGridLayout;
class QItemSelection;
class QLabel;
class QPushButton;
class QTableWidget;

/***************************************************************************
 Class for displaying goto/airlift dialog
***************************************************************************/
class goto_dialog: public qfc_dialog
{
  Q_OBJECT
  QTableWidget *goto_tab;
  QPushButton *goto_city;
  QPushButton *airlift_city;
  QPushButton *close_but;
  QCheckBox *show_all;
  QGridLayout *layout;
  QLabel *show_all_label;

public:
  goto_dialog(QWidget *parent = 0);
  void init();
  ~goto_dialog();
  void update_dlg();
  void show_me();
  void sort_def();

public slots:
  void close_dlg();

private slots:
  void go_to_city();
  void airlift_to();
  void item_selected(const QItemSelection &sl, const QItemSelection &ds);
  void checkbox_changed(Qt::CheckState state);
  void checkbox_changed_depr(int state);
protected:
  void paint(QPainter *painter, QPaintEvent *event);
  void paintEvent(QPaintEvent *event);

private:
  void fill_tab(struct player *pplayer);
  struct tile *original_tile;
};

#endif // FC__GOTODLG_H
