#include "db-controller-page.h"

struct _DbControllerPage
{
  ScNavigationPage parent_instance;
};

G_DEFINE_FINAL_TYPE (DbControllerPage, db_controller_page, SC_TYPE_NAVIGATION_PAGE)

static void
db_controller_page_class_init (DbControllerPageClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/hu/irl/sysex-controls/drumbrute/db-controller-page.ui");
}

static void
db_controller_page_init (DbControllerPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
