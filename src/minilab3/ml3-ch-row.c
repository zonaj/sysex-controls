#include "ml3-ch-row.h"

struct _Ml3ChRow
{
  ScComboRow parent_instance;
};

G_DEFINE_FINAL_TYPE (Ml3ChRow, ml3_ch_row, SC_TYPE_COMBO_ROW)

static void
ml3_ch_row_class_init (Ml3ChRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/hu/irl/sysex-controls/minilab3/ml3-ch-row.ui");
}

static void
ml3_ch_row_init (Ml3ChRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}