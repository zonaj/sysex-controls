#include "ar-control.h"

#include "ar-book.h"
#include "ar2-user-scale.h"
#include "sc-control.h"
#include "sc-control-value.h"
#include "sc-navigation-page.h"
#include "sc-preferences-page.h"
#include "sc-preferences-group.h"
#include "sc-window.h"

enum {
  PROP_0,
  PROP_ID,
  PROP_USE_CC_OFFSET,
  LAST_PROP,
};

struct _ArControl
{
  AdwBin parent_instance;
  uint32_t id;
  gboolean use_cc_offset;
  uint32_t real_id;
  uint8_t value;
  GtkWidget *widget;
};

static GParamSpec *value_props[LAST_PROP];

static void ar_control_interface_init (ScControlInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (ArControl, ar_control, ADW_TYPE_BIN,
                               G_IMPLEMENT_INTERFACE (SC_TYPE_CONTROL, ar_control_interface_init))

uint32_t
ar_control_get_id (ArControl *self)
{
  g_return_val_if_fail (AR_IS_CONTROL (self), 0);
  return self->id;
}

gboolean
ar_control_get_use_cc_offset (ArControl *self)
{
  g_return_val_if_fail (AR_IS_CONTROL (self), 0);
  return self->use_cc_offset;
}

static void
ar_control_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  ArControl *self = AR_CONTROL (object);

  switch (prop_id)
    {
    case PROP_ID:
      g_value_set_uint (value, ar_control_get_id (self));
    break;
    case PROP_USE_CC_OFFSET:
      g_value_set_uint (value, ar_control_get_use_cc_offset (self));
    break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
ar_control_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  ArControl *self = AR_CONTROL (object);

  switch (prop_id)
    {
    case PROP_ID:
      self->id = g_value_get_uint (value);
    break;
    case PROP_USE_CC_OFFSET:
      self->use_cc_offset = g_value_get_boolean (value);
    break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ar_control_class_init (ArControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = ar_control_get_property;
  object_class->set_property = ar_control_set_property;

  value_props[PROP_ID] = g_param_spec_uint ("id", NULL, NULL,
                                            0, G_MAXUINT32, 0,
                                            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  value_props[PROP_USE_CC_OFFSET] = g_param_spec_boolean ("use-cc-offset", NULL, NULL,
                                                          0,
                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, LAST_PROP, value_props);
}

static void
combo_row_change_cb (GObject * widget, GParamSpec *pspec, ArControl *self)
{
  ScWindow *window = SC_WINDOW (gtk_widget_get_root (GTK_WIDGET (self->widget)));
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  ScControlValue *item = SC_CONTROL_VALUE (adw_combo_row_get_selected_item (ADW_COMBO_ROW (widget)));
  uint8_t val = sc_control_value_get_value (item);

  if (self->value == val)
    return;

  g_debug ("combo control change 0x%08x: 0x%02x -> 0x%02x %s", self->real_id, self->value, val, sc_control_value_get_name (item));
  if (ar_book_write_control (book, self->real_id, val) < 0)
  {
    sc_io_problem (window, "Control change failed");
    return;
  }

  self->value = val;
}

static void
switch_row_change_cb (GObject * widget, GParamSpec *pspec, ArControl *self)
{
  ScWindow *window = SC_WINDOW (gtk_widget_get_root (GTK_WIDGET (self->widget)));
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  AdwSwitchRow *w = ADW_SWITCH_ROW (widget);
  uint8_t val = adw_switch_row_get_active (w);

  if (self->value == val)
    return;

  g_debug ("switch control change 0x%08x: 0x%02x -> 0x%02x", self->real_id, self->value, val);
  if (ar_book_write_control (book, self->real_id, val) < 0)
  {
    sc_io_problem (window, "Control change failed");
    return;
  }

  self->value = val;
}

static void
spin_row_change_cb (GObject * widget, GParamSpec *pspec, ArControl *self)
{
  ScWindow *window = SC_WINDOW (gtk_widget_get_root (GTK_WIDGET (self->widget)));
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  AdwSpinRow *w = ADW_SPIN_ROW (self->widget);
  uint8_t val = (uint8_t)adw_spin_row_get_value (w);

  if (self->value == val)
    return;

  g_debug("spin control change 0x%08x: 0x%02x -> 0x%02x", self->real_id, self->value, val);

  if (ar_book_write_control (book, self->real_id, val) < 0)
  {
    sc_io_problem (window, "Control change failed");

    /*
     * When setting page replaced by search page this spins forever
     * toggle sensitivity to stop, possibly a GTK bug?
     *
     * Now it emit this warning:
     * GtkText - did not receive a focus-out event.
     * If you handle this event, you must return
     * GDK_EVENT_PROPAGATE so the default handler gets the event as well
     */
    gtk_widget_set_sensitive (GTK_WIDGET (w), false);
    gtk_widget_set_sensitive (GTK_WIDGET (w), true);

    return;
  }

  self->value = val;
}

static void
toggle_button_change_cb (GObject * widget, GParamSpec *pspec, ArControl *self)
{
  ScWindow *window = SC_WINDOW (gtk_widget_get_root (GTK_WIDGET (self->widget)));
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  GtkToggleButton *b = GTK_TOGGLE_BUTTON (widget);
  uint8_t val = gtk_toggle_button_get_active (b) ? 0x7F : 00;

  if (self->value == val)
    return;

  g_debug ("toggle button change 0x%08x: 0x%02x -> 0x%02x", self->real_id, self->value, val);
  if (ar_book_write_control (book, self->real_id, val) < 0)
  {
    sc_io_problem (window, "Control change failed");
    return;
  }

  self->value = val;
}

static void
drop_down_change_cb (GObject * widget, GParamSpec *pspec, ArControl *self)
{
  ScWindow *window = SC_WINDOW (gtk_widget_get_root (GTK_WIDGET (self->widget)));
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  ScControlValue *item = SC_CONTROL_VALUE (gtk_drop_down_get_selected_item (GTK_DROP_DOWN (widget)));
  uint8_t val = sc_control_value_get_value (item);

  if (self->value == val)
    return;

  g_debug ("drop down change 0x%08x: 0x%02x -> 0x%02x %s", self->real_id, self->value, val, sc_control_value_get_name (item));
  if (ar_book_write_control (book, self->real_id, val) < 0)
  {
    sc_io_problem (window, "Control change failed");
    return;
  }

  self->value = val;
}

static void
user_scale_change_cb (GObject * widget, GParamSpec *pspec, ArControl *self)
{
  ScWindow *window = SC_WINDOW (gtk_widget_get_root (GTK_WIDGET (self->widget)));
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  Ar2UserScale *w = AR2_USER_SCALE (widget);
  uint8_t val = ar2_user_scale_get_value (w);

  if (self->value == val)
    return;

  g_debug ("user scale part change 0x%08x: 0x%02x -> 0x%02x (part %d)", self->real_id, self->value, val, ar2_user_scale_get_part (w));
  if (ar_book_write_control (book, self->real_id, val) < 0)
  {
    sc_io_problem (window, "Control change failed");
    return;
  }

  self->value = val;
}

static int
ar_control_register (ArControl *self)
{
  ScPreferencesGroup *group_widget;
  ScPreferencesPage *page_widget;
  ScNavigationPage *nav_page_widget;
  GtkWidget *widget;
  uint8_t *rid = (uint8_t*)&(self->real_id);
  self->real_id = self->id;

  widget = gtk_widget_get_ancestor (GTK_WIDGET (self), ADW_TYPE_COMBO_ROW);

  if (!widget)
    widget = gtk_widget_get_ancestor (GTK_WIDGET (self), ADW_TYPE_SPIN_ROW);

  if (!widget)
    widget = gtk_widget_get_ancestor (GTK_WIDGET (self), ADW_TYPE_SWITCH_ROW);

  if (!widget && GTK_IS_BOX (gtk_widget_get_parent (GTK_WIDGET (self))))
    widget = gtk_widget_get_next_sibling (GTK_WIDGET (self));

  nav_page_widget = SC_NAVIGATION_PAGE (gtk_widget_get_ancestor (GTK_WIDGET (&self->parent_instance), SC_TYPE_NAVIGATION_PAGE));
  if (nav_page_widget) {
    uint32_t cc_offset = sc_navigation_page_get_control_cc_offset (nav_page_widget);
    self->real_id += self->use_cc_offset && cc_offset ? cc_offset : sc_navigation_page_get_control_id_offset (nav_page_widget);
  }

  page_widget = SC_PREFERENCES_PAGE (gtk_widget_get_ancestor (GTK_WIDGET (&self->parent_instance), SC_TYPE_PREFERENCES_PAGE));
  if (page_widget) {
    uint32_t cc_offset = sc_preferences_page_get_control_cc_offset (page_widget);
    self->real_id += self->use_cc_offset && cc_offset ? cc_offset : sc_preferences_page_get_control_id_offset (page_widget);
  }

  group_widget = SC_PREFERENCES_GROUP (gtk_widget_get_ancestor (GTK_WIDGET (&self->parent_instance), SC_TYPE_PREFERENCES_GROUP));
  if (group_widget) {
    uint32_t cc_offset = sc_preferences_group_get_control_cc_offset (group_widget);
    self->real_id += self->use_cc_offset && cc_offset ? cc_offset : sc_preferences_group_get_control_id_offset (group_widget);
  }

  /* normalize
   * after adding the offsets, it is possible that cid >= 0x80
   * sysex bytes are < 0x80
   * arturia v3 uses the last byte of the ID (rid) for storing the overflow bit
   * */

  if (rid[1] >= 0x80) {
    rid[0]++;
    rid[1] -= 0x80;
  }

  self->value = 0;
  self->widget = widget;
  sc_navigation_page_register_control (nav_page_widget, SC_CONTROL (self));

  if (ADW_IS_COMBO_ROW (widget))
    g_signal_connect (G_OBJECT (widget), "notify::selected-item", G_CALLBACK (combo_row_change_cb), self);
  else if (ADW_IS_SWITCH_ROW (widget))
    g_signal_connect (G_OBJECT (widget), "notify::active", G_CALLBACK (switch_row_change_cb), self);
  else if (ADW_IS_SPIN_ROW (widget))
    g_signal_connect (G_OBJECT (widget), "notify::value", G_CALLBACK (spin_row_change_cb), self);
  else if (GTK_IS_TOGGLE_BUTTON (widget))
    g_signal_connect (G_OBJECT (widget), "notify::active", G_CALLBACK (toggle_button_change_cb), self);
  else if (GTK_IS_DROP_DOWN (widget))
    g_signal_connect (G_OBJECT (widget), "notify::selected", G_CALLBACK (drop_down_change_cb), self);
  else if (AR2_IS_USER_SCALE (widget))
    g_signal_connect (G_OBJECT (widget), "notify::value", G_CALLBACK (user_scale_change_cb), self);
  else
    g_error("Unsupported control type: %s id: 0x%08x",
            gtk_widget_get_name (GTK_WIDGET (widget)),
            self->real_id);

  return false;
}

static void
ar_control_update_gui (ScControl *control)
{
  ArControl *self = AR_CONTROL (control);

  if (ADW_IS_COMBO_ROW (self->widget))
  {
    guint pos = GTK_INVALID_LIST_POSITION;
    AdwComboRow* combo_row = ADW_COMBO_ROW (self->widget);
    GListModel* list = adw_combo_row_get_model (combo_row);
    for(int i = 0; i < g_list_model_get_n_items (list); ++i)
    {
      ScControlValue *kv = SC_CONTROL_VALUE (g_list_model_get_item (list, i));
      if (sc_control_value_get_value (kv) == self->value)
      {
        pos = i;
        break;
      }
    }

    if (pos != GTK_INVALID_LIST_POSITION)
      adw_combo_row_set_selected (combo_row, pos);
    else
      g_warning("Set combo row id 0x%02x to invalid pos 0x%02x (value: 0x%02x)", self->real_id, pos, self->value);

    g_debug("Set combo row id 0x%02x to pos 0x%02x (value: 0x%02x)", self->real_id, pos, self->value);
  }
  else if (ADW_IS_SWITCH_ROW (self->widget))
  {
    AdwSwitchRow *switch_row = ADW_SWITCH_ROW (self->widget);
    adw_switch_row_set_active (switch_row, self->value);
    g_debug ("Set switch row with id 0x%02x to 0x%02x", self->real_id, self->value);
  }
  else if (ADW_IS_SPIN_ROW (self->widget))
  {
    AdwSpinRow *spin_row = ADW_SPIN_ROW (self->widget);
    adw_spin_row_set_value (spin_row, self->value);
    g_debug ("Set spin row with id 0x%02x to 0x%02x", self->real_id, self->value);
  }
  else if (GTK_IS_TOGGLE_BUTTON (self->widget))
  {
    GtkToggleButton *b = GTK_TOGGLE_BUTTON (self->widget);
    gtk_toggle_button_set_active (b, self->value);
    g_debug ("Set toggle button with id 0x%02x to 0x%02x", self->real_id, self->value);
  }
  else if (GTK_IS_DROP_DOWN (self->widget))
  {
    guint pos = GTK_INVALID_LIST_POSITION;
    GtkDropDown *dd = GTK_DROP_DOWN (self->widget);
    GListModel* list = gtk_drop_down_get_model (dd);
    for(int i = 0; i < g_list_model_get_n_items (list); ++i)
    {
      ScControlValue *kv = SC_CONTROL_VALUE (g_list_model_get_item (list, i));
      if (sc_control_value_get_value (kv) == self->value)
      {
        pos = i;
        break;
      }
    }

    if (pos != GTK_INVALID_LIST_POSITION)
      gtk_drop_down_set_selected (dd, pos);
    else
      g_warning("Set drop down id 0x%02x to invalid pos 0x%02x (value 0x%02x not found)", self->real_id, pos, self->value);

    g_debug("Set dop_down id 0x%02x to pos 0x%02x (value: 0x%02x)", self->real_id, pos, self->value);
  }
  else if (AR2_IS_USER_SCALE (self->widget))
  {
    Ar2UserScale *w = AR2_USER_SCALE (self->widget);
    ar2_user_scale_set_value (w, self->value);
    g_debug ("Set user scale part with id 0x%02x to 0x%02x (part %d)", self->real_id, self->value, ar2_user_scale_get_part (w));
  }
  else
  {
    g_error ("Unsupported control type: %s id: 0x%02x",
              gtk_widget_get_name (GTK_WIDGET (self->widget)),
              self->real_id);
  }
}

static int
ar_control_read_value (ScControl *control)
{
  ArControl *self = AR_CONTROL (control);
  ArBook *book = AR_BOOK (gtk_widget_get_ancestor (GTK_WIDGET (self->widget), AR_TYPE_BOOK));
  return ar_book_read_control (book, self->real_id, &self->value);
}

static void
ar_control_interface_init (ScControlInterface *iface)
{
  iface->update_gui = ar_control_update_gui;
  iface->read_value = ar_control_read_value;
}

static void
ar_control_init (ArControl *self)
{
  gtk_widget_set_visible (GTK_WIDGET (&self->parent_instance), false);
  g_idle_add (G_SOURCE_FUNC (ar_control_register), self);
}
