#include "kles3-fader.h"

struct _Kles3Fader
{
  ScPreferencesGroup parent_instance;
};

G_DEFINE_FINAL_TYPE (Kles3Fader, kles3_fader, SC_TYPE_PREFERENCES_GROUP)

static void
kles3_fader_class_init (Kles3FaderClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/hu/irl/sysex-controls/keylabes3/kles3-fader.ui");
}

static void
kles3_fader_init (Kles3Fader *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
