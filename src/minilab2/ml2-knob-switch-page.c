#include "ml2-knob-switch-page.h"

struct _Ml2KnobSwitchPage
{
  ScNavigationPage parent_instance;
};

G_DEFINE_FINAL_TYPE (Ml2KnobSwitchPage, ml2_knob_switch_page, SC_TYPE_NAVIGATION_PAGE)

static void
ml2_knob_switch_page_class_init (Ml2KnobSwitchPageClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/hu/irl/sysex-controls/minilab2/ml2-knob-switch-page.ui");
}

static void
ml2_knob_switch_page_init (Ml2KnobSwitchPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
