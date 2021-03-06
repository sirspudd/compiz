/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */

/*
 * Copyright (C) 2006 Novell, Inc.
 * Copyright (C) 2010 Sam Spilsbury
 * Copyright (C) 2011 Canonical Ltd.
 * Copyright (C) 2016 Alberts Muktupāvels
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 *     David Reveman <davidr@novell.com>
 *     Sam Spilsbury <smspillaz@gmail.com>
 */

#include "config.h"

#include <libmetacity/meta-theme.h>

#include "gtk-window-decorator.h"
#include "gwd-settings.h"
#include "gwd-theme-metacity.h"

struct _GWDThemeMetacity
{
    GWDTheme   parent;

    MetaTheme *theme;

    gulong     button_layout_id;
};

G_DEFINE_TYPE (GWDThemeMetacity, gwd_theme_metacity, GWD_TYPE_THEME)

static MetaFrameType
frame_type_from_string (const gchar *str)
{
    if (strcmp ("dialog", str) == 0)
        return META_FRAME_TYPE_DIALOG;
    else if (strcmp ("modal_dialog", str) == 0)
        return META_FRAME_TYPE_MODAL_DIALOG;
    else if (strcmp ("utility", str) == 0)
        return META_FRAME_TYPE_UTILITY;
    else if (strcmp ("menu", str) == 0)
        return META_FRAME_TYPE_MENU;

    return META_FRAME_TYPE_NORMAL;
}

static void
update_metacity_button_layout_cb (GWDSettings      *settings,
                                  const gchar      *button_layout,
                                  GWDThemeMetacity *metacity)
{
    gboolean invert = gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL;

    meta_theme_set_button_layout (metacity->theme, button_layout, invert);
}

static MetaButtonState
meta_button_state (gint state)
{
    if (state & IN_EVENT_WINDOW) {
        if (state & PRESSED_EVENT_WINDOW)
            return META_BUTTON_STATE_PRESSED;

        return META_BUTTON_STATE_PRELIGHT;
    }

    return META_BUTTON_STATE_NORMAL;
}

static MetaButtonState
meta_button_state_for_button_type (decor_t        *decor,
                                   MetaButtonType  type)
{
    switch (type) {
        case META_BUTTON_TYPE_CLOSE:
            return meta_button_state (decor->button_states[BUTTON_CLOSE]);
        case META_BUTTON_TYPE_MAXIMIZE:
            return meta_button_state (decor->button_states[BUTTON_MAX]);
        case META_BUTTON_TYPE_MINIMIZE:
            return meta_button_state (decor->button_states[BUTTON_MIN]);
        case META_BUTTON_TYPE_MENU:
            return meta_button_state (decor->button_states[BUTTON_MENU]);
        default:
            break;
    }

    return META_BUTTON_STATE_NORMAL;
}

static MetaButtonState
update_button_state (MetaButtonType type,
                     GdkRectangle   rect,
                     gpointer       user_data)
{
    decor_t *decor = (decor_t *) user_data;

    return meta_button_state_for_button_type (decor, type);
}

static gint
radius_to_width (gint radius,
                 gint i)
{
    gfloat r1 = sqrt (radius) + radius;
    gfloat r2 = r1 * r1 - (r1 - (i + 0.5)) * (r1 - (i + 0.5));

    return floor (0.5f + r1 - sqrt (r2));
}

static void
get_corner_radius (const MetaFrameGeometry *fgeom,
                   gint                    *top_left_radius,
                   gint                    *top_right_radius,
                   gint                    *bottom_left_radius,
                   gint                    *bottom_right_radius)
{
    *top_left_radius = fgeom->top_left_corner_rounded_radius;
    *top_right_radius = fgeom->top_right_corner_rounded_radius;
    *bottom_left_radius = fgeom->bottom_left_corner_rounded_radius;
    *bottom_right_radius = fgeom->bottom_right_corner_rounded_radius;
}

static Region
get_top_border_region (const MetaFrameGeometry *fgeom,
                       gboolean                 subtract_borders)
{
    GtkBorder invisible = fgeom->borders.invisible;
    GtkBorder shadow = fgeom->borders.shadow;
    Region corners_xregion;
    Region border_xregion;
    XRectangle xrect;
    gint top_left_radius;
    gint top_right_radius;
    gint bottom_left_radius;
    gint bottom_right_radius;
    gint w;
    gint i;
    gint width;
    gint height;

    corners_xregion = XCreateRegion ();

    get_corner_radius (fgeom, &top_left_radius, &top_right_radius,
                       &bottom_left_radius, &bottom_right_radius);

    width = fgeom->width - invisible.left - invisible.right + shadow.left + shadow.right;
    height = fgeom->borders.visible.top + shadow.top;

    if (top_left_radius && subtract_borders) {
        for (i = 0; i < top_left_radius; ++i) {
            w = radius_to_width (top_left_radius, i);

            xrect.x = 0;
            xrect.y = i;
            xrect.width = w;
            xrect.height = 1;

            XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
        }
    }

    if (top_right_radius && subtract_borders) {
        for (i = 0; i < top_right_radius; ++i) {
            w = radius_to_width (top_right_radius, i);

            xrect.x = width - w;
            xrect.y = i;
            xrect.width = w;
            xrect.height = 1;

            XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
        }
    }

    border_xregion = XCreateRegion ();

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = width;
    xrect.height = height;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    XSubtractRegion (border_xregion, corners_xregion, border_xregion);
    XDestroyRegion (corners_xregion);

    return border_xregion;
}

static Region
get_bottom_border_region (const MetaFrameGeometry *fgeom,
                          gboolean                 subtract_borders)
{
    GtkBorder invisible = fgeom->borders.invisible;
    GtkBorder shadow = fgeom->borders.shadow;
    Region corners_xregion;
    Region border_xregion;
    XRectangle xrect;
    gint top_left_radius;
    gint top_right_radius;
    gint bottom_left_radius;
    gint bottom_right_radius;
    gint w;
    gint i;
    gint width;
    gint height;

    corners_xregion = XCreateRegion ();

    get_corner_radius (fgeom, &top_left_radius, &top_right_radius,
                       &bottom_left_radius, &bottom_right_radius);

    width = fgeom->width - invisible.left - invisible.right + shadow.left + shadow.right;
    height = fgeom->borders.visible.bottom + shadow.bottom;

    if (bottom_left_radius && subtract_borders) {
        for (i = 0; i < bottom_left_radius; ++i) {
            w = radius_to_width (bottom_left_radius, i);

            xrect.x = 0;
            xrect.y = height - i - 1;
            xrect.width = w;
            xrect.height = 1;

            XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
        }
    }

    if (bottom_right_radius && subtract_borders) {
        for (i = 0; i < bottom_right_radius; ++i) {
            w = radius_to_width (bottom_right_radius, i);

            xrect.x = width - w;
            xrect.y = height - i - 1;
            xrect.width = w;
            xrect.height = 1;

            XUnionRectWithRegion (&xrect, corners_xregion, corners_xregion);
        }
    }

    border_xregion = XCreateRegion ();

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = width;
    xrect.height = height;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    XSubtractRegion (border_xregion, corners_xregion, border_xregion);
    XDestroyRegion (corners_xregion);

    return border_xregion;
}

static Region
get_left_border_region (const MetaFrameGeometry *fgeom)
{
    Region border_xregion;
    XRectangle xrect;

    border_xregion = XCreateRegion ();

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = fgeom->borders.visible.left + fgeom->borders.shadow.left;
    xrect.height = fgeom->height - fgeom->borders.total.top - fgeom->borders.total.bottom;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    return border_xregion;
}

static Region
get_right_border_region (const MetaFrameGeometry *fgeom)
{
    Region border_xregion;
    XRectangle xrect;

    border_xregion = XCreateRegion ();

    xrect.x = 0;
    xrect.y = 0;
    xrect.width = fgeom->borders.visible.right + fgeom->borders.shadow.right;
    xrect.height = fgeom->height - fgeom->borders.total.top - fgeom->borders.total.bottom;

    XUnionRectWithRegion (&xrect, border_xregion, border_xregion);

    return border_xregion;
}

static void
decor_update_meta_window_property (GWDThemeMetacity *metacity,
                                   decor_t          *d,
                                   MetaFrameFlags    flags,
                                   MetaFrameType     type,
                                   Region            top,
                                   Region            bottom,
                                   Region            left,
                                   Region            right)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = gdk_x11_display_get_xdisplay (display);
    unsigned int frame_type = populate_frame_type (d);
    unsigned int frame_state = populate_frame_state (d);
    unsigned int frame_actions = populate_frame_actions (d);
    unsigned int nOffset = 1;
    decor_extents_t win_extents;
    decor_extents_t frame_win_extents;
    decor_extents_t max_win_extents;
    decor_extents_t frame_max_win_extents;
    decor_quad_t quads[N_QUADS_MAX];
    gint w;
    gint lh;
    gint rh;
    gint top_stretch_offset;
    gint bottom_stretch_offset;
    gint left_stretch_offset;
    gint right_stretch_offset;
    gint nQuad;
    long *data;

    win_extents = frame_win_extents = d->frame->win_extents;
    max_win_extents = frame_max_win_extents = d->frame->max_win_extents;

    /* Add the invisible grab area padding */
    {
        MetaFrameFlags tmp_flags;
        MetaFrameBorders borders;

        tmp_flags = flags & ~META_FRAME_MAXIMIZED;
        meta_theme_get_frame_borders (metacity->theme, d->gtk_theme_variant,
                                      type, tmp_flags, &borders);

        frame_win_extents.left += borders.resize.left;
        frame_win_extents.right += borders.resize.right;
        frame_win_extents.bottom += borders.resize.bottom;
        frame_win_extents.top += borders.resize.top;

        tmp_flags = flags | META_FRAME_MAXIMIZED;
        meta_theme_get_frame_borders (metacity->theme, d->gtk_theme_variant,
                                      type, tmp_flags, &borders);

        frame_max_win_extents.left += borders.resize.left;
        frame_max_win_extents.right += borders.resize.right;
        frame_max_win_extents.bottom += borders.resize.bottom;
        frame_max_win_extents.top += borders.resize.top;
    }

    w = d->border_layout.top.x2 - d->border_layout.top.x1 -
        d->context->left_space - d->context->right_space;

    if (d->border_layout.rotation)
        lh = d->border_layout.left.x2 - d->border_layout.left.x1;
    else
        lh = d->border_layout.left.y2 - d->border_layout.left.y1;

    if (d->border_layout.rotation)
        rh = d->border_layout.right.x2 - d->border_layout.right.x1;
    else
        rh = d->border_layout.right.y2 - d->border_layout.right.y1;

    left_stretch_offset = lh / 2;
    right_stretch_offset = rh / 2;
    top_stretch_offset = w - d->button_width - 1;
    bottom_stretch_offset = (d->border_layout.bottom.x2 - d->border_layout.bottom.x1 -
                             d->context->left_space - d->context->right_space) / 2;

    nQuad = decor_set_lXrXtXbX_window_quads (quads, d->context, &d->border_layout,
                                             left_stretch_offset, right_stretch_offset,
                                             top_stretch_offset, bottom_stretch_offset);

    data = decor_alloc_property (nOffset, WINDOW_DECORATION_TYPE_PIXMAP);
    decor_quads_to_property (data, nOffset - 1, cairo_xlib_surface_get_drawable (d->surface),
                             &frame_win_extents, &win_extents,
                             &frame_max_win_extents, &max_win_extents,
                             ICON_SPACE + d->button_width,
                             0, quads, nQuad, frame_type, frame_state, frame_actions);

    gdk_error_trap_push ();

    XChangeProperty (xdisplay, d->prop_xid, win_decor_atom, XA_INTEGER,
                     32, PropModeReplace, (guchar *) data,
                     PROP_HEADER_SIZE + BASE_PROP_SIZE + QUAD_PROP_SIZE * N_QUADS_MAX);
    gdk_display_sync (display);

    gdk_error_trap_pop_ignored ();

    free (data);

    decor_update_blur_property (d, w, lh,
                                top, top_stretch_offset,
                                bottom, bottom_stretch_offset,
                                left, left_stretch_offset,
                                right, right_stretch_offset);
}

static void
get_decoration_geometry (GWDThemeMetacity  *metacity,
                         decor_t           *decor,
                         MetaFrameFlags    *flags,
                         MetaFrameGeometry *fgeom,
                         MetaFrameType      frame_type)
{
    gint client_width;
    gint client_height;

    if (!(frame_type < META_FRAME_TYPE_LAST))
        frame_type = META_FRAME_TYPE_NORMAL;

    *flags = 0;

    if (decor->actions & WNCK_WINDOW_ACTION_CLOSE)
        *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_DELETE;

    if (decor->actions & WNCK_WINDOW_ACTION_MINIMIZE)
        *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MINIMIZE;

    if (decor->actions & WNCK_WINDOW_ACTION_MAXIMIZE)
        *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MAXIMIZE;

    *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MENU;

    if (decor->actions & WNCK_WINDOW_ACTION_RESIZE) {
        if (!(decor->state & WNCK_WINDOW_STATE_MAXIMIZED_VERTICALLY))
            *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_VERTICAL_RESIZE;

        if (!(decor->state & WNCK_WINDOW_STATE_MAXIMIZED_HORIZONTALLY))
            *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_HORIZONTAL_RESIZE;
    }

    if (decor->actions & WNCK_WINDOW_ACTION_MOVE)
        *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MOVE;

    if (decor->actions & WNCK_WINDOW_ACTION_MAXIMIZE)
        *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_MAXIMIZE;

    if (decor->actions & WNCK_WINDOW_ACTION_SHADE)
        *flags |= (MetaFrameFlags ) META_FRAME_ALLOWS_SHADE;

    if (decor->active)
        *flags |= (MetaFrameFlags ) META_FRAME_HAS_FOCUS;

    if ((decor->state & META_MAXIMIZED) == META_MAXIMIZED)
        *flags |= (MetaFrameFlags ) META_FRAME_MAXIMIZED;

    if (decor->state & WNCK_WINDOW_STATE_STICKY)
        *flags |= (MetaFrameFlags ) META_FRAME_STUCK;

    if (decor->state & WNCK_WINDOW_STATE_FULLSCREEN)
        *flags |= (MetaFrameFlags ) META_FRAME_FULLSCREEN;

    if (decor->state & WNCK_WINDOW_STATE_SHADED)
        *flags |= (MetaFrameFlags ) META_FRAME_SHADED;

    if (decor->state & WNCK_WINDOW_STATE_ABOVE)
        *flags |= (MetaFrameFlags ) META_FRAME_ABOVE;

    client_width = decor->border_layout.top.x2 - decor->border_layout.top.x1;
    client_width -= decor->context->right_space + decor->context->left_space;

    if (decor->border_layout.rotation)
        client_height = decor->border_layout.left.x2 - decor->border_layout.left.x1;
    else
        client_height = decor->border_layout.left.y2 - decor->border_layout.left.y1;

    meta_theme_calc_geometry (metacity->theme, decor->gtk_theme_variant,
                              frame_type, *flags, client_width, client_height,
                              fgeom);
}

static void
calc_button_size (GWDTheme *theme,
                  decor_t  *decor)
{
    MetaFrameType frame_type;
    MetaFrameFlags flags;
    MetaFrameGeometry fgeom;
    gint i, min_x, x, y, w, h, width;

    if (!decor->context) {
        decor->button_width = 0;
        return;
    }

    frame_type = frame_type_from_string (decor->frame->type);

    get_decoration_geometry (GWD_THEME_METACITY (theme), decor, &flags,
                             &fgeom, frame_type);

    width = decor->border_layout.top.x2 - decor->border_layout.top.x1 -
            decor->context->left_space - decor->context->right_space +
            fgeom.borders.total.left + fgeom.borders.total.right;

    min_x = width;

    for (i = 0; i < 3; ++i) {
        static guint button_actions[3] = {
            WNCK_WINDOW_ACTION_CLOSE,
            WNCK_WINDOW_ACTION_MAXIMIZE,
            WNCK_WINDOW_ACTION_MINIMIZE
        };

        if (decor->actions & button_actions[i]) {
            if (gwd_theme_get_button_position (theme, decor, i, width, 256,
                                               &x, &y, &w, &h)) {
                if (x > width / 2 && x < min_x)
                    min_x = x;
            }
        }
    }

    decor->button_width = width - min_x;
}

static MetaButtonType
button_type_to_meta_button_type (gint button_type)
{
    switch (button_type) {
        case BUTTON_MENU:
            return META_BUTTON_TYPE_MENU;
        case BUTTON_MIN:
            return META_BUTTON_TYPE_MINIMIZE;
        case BUTTON_MAX:
            return META_BUTTON_TYPE_MAXIMIZE;
        case BUTTON_CLOSE:
            return META_BUTTON_TYPE_CLOSE;
        default:
            break;
    }

    return META_BUTTON_TYPE_LAST;
}

static gboolean
setup_theme (GWDThemeMetacity *metacity)
{
    GWDSettings *settings = gwd_theme_get_settings (GWD_THEME (metacity));
    const gchar *metacity_theme_name = gwd_settings_get_metacity_theme_name (settings);
    gint metacity_theme_type = gwd_settings_get_metacity_theme_type (settings);
    GError *error = NULL;

    /* metacity_theme can be NULL only in one case - if user has disabled
     * metacity theme with use-metacity-theme setting. In that case
     * GWDThemeCairo will be created / should be created.
     */
    g_assert (metacity_theme_name != NULL);

    if (metacity_theme_type == -1)
        metacity_theme_type = META_THEME_TYPE_METACITY;

    metacity->theme = meta_theme_new (metacity_theme_type);

    if (!meta_theme_load (metacity->theme, metacity_theme_name, &error)) {
        g_warning ("Failed to load metacity theme '%s': %s",
                   metacity_theme_name, error->message);

        g_error_free (error);
        g_clear_object (&metacity->theme);

        return FALSE;
    }

    return TRUE;
}

static void
setup_button_layout (GWDThemeMetacity *metacity)
{
    GWDSettings *settings = gwd_theme_get_settings (GWD_THEME (metacity));
    const gchar *button_layout = gwd_settings_get_metacity_button_layout (settings);

    metacity->button_layout_id =
        g_signal_connect (settings, "update-metacity-button-layout",
                          G_CALLBACK (update_metacity_button_layout_cb), metacity);

    update_metacity_button_layout_cb (settings, button_layout, metacity);
}

static void
gwd_theme_metacity_constructed (GObject *object)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (object);

    G_OBJECT_CLASS (gwd_theme_metacity_parent_class)->constructed (object);

    if (!setup_theme (metacity))
        return;

    setup_button_layout (metacity);

#ifdef HAVE_METACITY_3_26_0
    meta_theme_set_scale (metacity->theme, gwd_theme_get_scale (GWD_THEME (object)));
    meta_theme_set_dpi (metacity->theme, gwd_theme_get_dpi (GWD_THEME (object)));
#endif
}

static void
gwd_theme_metacity_dispose (GObject *object)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (object);

    g_clear_object (&metacity->theme);

    if (metacity->button_layout_id != 0) {
        GWDSettings *settings = gwd_theme_get_settings (GWD_THEME (metacity));

        g_signal_handler_disconnect (settings, metacity->button_layout_id);
        metacity->button_layout_id = 0;
    }

    G_OBJECT_CLASS (gwd_theme_metacity_parent_class)->dispose (object);
}

static void
gwd_theme_metacity_dpi_changed (GWDTheme *theme)
{
#ifdef HAVE_METACITY_3_26_0
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);

    meta_theme_set_dpi (metacity->theme, gwd_theme_get_dpi (theme));
#endif
}

static void
gwd_theme_metacity_scale_changed (GWDTheme *theme)
{
#ifdef HAVE_METACITY_3_26_0
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);

    meta_theme_set_scale (metacity->theme, gwd_theme_get_scale (theme));
#endif
}

static void
gwd_theme_metacity_style_updated (GWDTheme *theme)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);

    meta_theme_invalidate (metacity->theme);
}

static void
gwd_theme_metacity_draw_window_decoration (GWDTheme *theme,
                                           decor_t  *decor)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);
    GWDSettings *settings = gwd_theme_get_settings (theme);
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = gdk_x11_display_get_xdisplay (display);
    GtkWidget *style_window = gwd_theme_get_style_window (theme);
    cairo_surface_t *surface;
    Picture src;
    MetaFrameGeometry fgeom;
    MetaFrameFlags flags;
    MetaFrameType frame_type;
    cairo_t *cr;
    Region top_region;
    Region bottom_region;
    Region left_region;
    Region right_region;
    double alpha;
    gboolean shade_alpha;

    if (!decor->surface || !decor->picture)
        return;

    top_region = NULL;
    bottom_region = NULL;
    left_region = NULL;
    right_region = NULL;

    if (decor->active) {
        alpha = gwd_settings_get_metacity_active_opacity (settings);
        shade_alpha = gwd_settings_get_metacity_active_shade_opacity (settings);
    } else {
        alpha = gwd_settings_get_metacity_inactive_opacity (settings);
        shade_alpha = gwd_settings_get_metacity_inactive_shade_opacity (settings);
    }

    if (decoration_alpha == 1.0)
        alpha = 1.0;

    cr = cairo_create (decor->buffer_surface ? decor->buffer_surface : decor->surface);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    frame_type = frame_type_from_string (decor->frame->type);

    get_decoration_geometry (metacity, decor, &flags, &fgeom, frame_type);

    if (!decor->frame->has_shadow_extents &&
        (decor->prop_xid || !decor->buffer_surface)) {
        draw_shadow_background (decor, cr, decor->shadow, decor->context);
    }

    /* Draw something that will be almost invisible to user. This is hacky way
     * to fix invisible decorations. */
    cairo_set_source_rgba (cr, 0, 0, 0, 0.01);
    cairo_rectangle (cr, 0, 0, 1, 1);
    cairo_fill (cr);
    /* ------------ */

    cairo_destroy (cr);

    surface = create_surface (fgeom.width, fgeom.height, style_window);
    cairo_surface_set_device_scale (surface, 1, 1);
    cr = cairo_create (surface);

    src = XRenderCreatePicture (xdisplay, cairo_xlib_surface_get_drawable (surface),
                                xformat_rgba, 0, NULL);

    meta_theme_draw_frame (metacity->theme, decor->gtk_theme_variant, cr, frame_type, flags,
                           fgeom.width - fgeom.borders.total.left - fgeom.borders.total.right,
                           fgeom.height - fgeom.borders.total.top - fgeom.borders.total.bottom,
                           decor->name, update_button_state, decor, decor->icon_pixbuf, NULL);

    if (fgeom.borders.visible.top + fgeom.borders.shadow.top) {
        top_region = get_top_border_region (&fgeom, !decor->frame->has_shadow_extents);

        decor_blend_border_picture (xdisplay, decor->context, src,
                                    fgeom.borders.invisible.left - fgeom.borders.shadow.left,
                                    fgeom.borders.invisible.top - fgeom.borders.shadow.top,
                                    decor->picture, &decor->border_layout,
                                    BORDER_TOP, top_region,
                                    alpha * 0xffff, shade_alpha, 0);
    }

    if (fgeom.borders.visible.bottom + fgeom.borders.shadow.bottom) {
        bottom_region = get_bottom_border_region (&fgeom, !decor->frame->has_shadow_extents);

        decor_blend_border_picture (xdisplay, decor->context, src,
                                    fgeom.borders.invisible.left - fgeom.borders.shadow.left,
                                    fgeom.height - fgeom.borders.total.bottom,
                                    decor->picture, &decor->border_layout,
                                    BORDER_BOTTOM, bottom_region,
                                    alpha * 0xffff, shade_alpha, 0);
    }

    if (fgeom.borders.visible.left + fgeom.borders.shadow.left) {
        left_region = get_left_border_region (&fgeom);

        decor_blend_border_picture (xdisplay, decor->context, src,
                                    fgeom.borders.invisible.left - fgeom.borders.shadow.left,
                                    fgeom.borders.total.top,
                                    decor->picture, &decor->border_layout,
                                    BORDER_LEFT, left_region,
                                    alpha * 0xffff, shade_alpha, 0);
    }

    if (fgeom.borders.visible.right + fgeom.borders.shadow.right) {
        right_region = get_right_border_region (&fgeom);

        decor_blend_border_picture (xdisplay, decor->context, src,
                                    fgeom.width - fgeom.borders.total.right,
                                    fgeom.borders.total.top,
                                    decor->picture, &decor->border_layout,
                                    BORDER_RIGHT, right_region,
                                    alpha * 0xffff, shade_alpha, 0);
    }

    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    XRenderFreePicture (xdisplay, src);

    copy_to_front_buffer (decor);

    if (decor->prop_xid) {
        /* translate from frame to client window space */
        if (top_region)
            XOffsetRegion (top_region, -fgeom.borders.total.left, -fgeom.borders.total.top);
        if (bottom_region)
            XOffsetRegion (bottom_region, -fgeom.borders.total.left, 0);
        if (left_region)
            XOffsetRegion (left_region, -fgeom.borders.total.left, 0);

        decor_update_meta_window_property (metacity, decor, flags, frame_type,
                                           top_region, bottom_region,
                                           left_region, right_region);

        decor->prop_xid = 0;
    }

    if (top_region)
        XDestroyRegion (top_region);
    if (bottom_region)
        XDestroyRegion (bottom_region);
    if (left_region)
        XDestroyRegion (left_region);
    if (right_region)
        XDestroyRegion (right_region);
}

static gboolean
gwd_theme_metacity_calc_decoration_size (GWDTheme *theme,
                                         decor_t  *decor,
                                         gint      w,
                                         gint      h,
                                         gint      name_width,
                                         gint     *width,
                                         gint     *height)
{
    decor_layout_t layout;
    decor_context_t *context;
    decor_shadow_t *shadow;

    if (!decor->decorated)
        return FALSE;

    if ((decor->state & META_MAXIMIZED) == META_MAXIMIZED) {
        if (decor->active) {
            context = &decor->frame->max_window_context_active;
            shadow = decor->frame->max_border_shadow_active;
        } else {
            context = &decor->frame->max_window_context_inactive;
            shadow = decor->frame->max_border_shadow_inactive;
        }
    } else {
        if (decor->active) {
            context = &decor->frame->window_context_active;
            shadow = decor->frame->border_shadow_active;
        } else {
            context = &decor->frame->window_context_inactive;
            shadow = decor->frame->border_shadow_inactive;
        }
    }

    decor_get_best_layout (context, w, h, &layout);

    if (context != decor->context ||
        memcmp (&layout, &decor->border_layout, sizeof (layout))) {
        *width = layout.width;
        *height = layout.height;

        decor->border_layout = layout;
        decor->context = context;
        decor->shadow = shadow;

        calc_button_size (theme, decor);

        return TRUE;
    }

    return FALSE;
}

static void
gwd_theme_metacity_update_border_extents (GWDTheme      *theme,
                                          decor_frame_t *frame)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);
    GWDSettings *settings = gwd_theme_get_settings (theme);
    gint theme_type = gwd_settings_get_metacity_theme_type (settings);
    MetaFrameType frame_type = frame_type_from_string (frame->type);
    MetaFrameBorders borders;

    gwd_decor_frame_ref (frame);

    /* Shadow extents is used only with GTK+ theme */
    frame->has_shadow_extents = theme_type == META_THEME_TYPE_GTK;

    meta_theme_get_frame_borders (metacity->theme, NULL, frame_type,
                                  0, &borders);

    frame->win_extents.top = borders.visible.top;
    frame->win_extents.bottom = borders.visible.bottom;
    frame->win_extents.left = borders.visible.left;
    frame->win_extents.right = borders.visible.right;
    frame->shadow_extents.top = borders.shadow.top;
    frame->shadow_extents.bottom = borders.shadow.bottom;
    frame->shadow_extents.left = borders.shadow.left;
    frame->shadow_extents.right = borders.shadow.right;

    meta_theme_get_frame_borders (metacity->theme, NULL, frame_type,
                                  META_FRAME_MAXIMIZED, &borders);

    frame->max_win_extents.top = borders.visible.top;
    frame->max_win_extents.bottom = borders.visible.bottom;
    frame->max_win_extents.left = borders.visible.left;
    frame->max_win_extents.right = borders.visible.right;
    frame->max_shadow_extents.top = borders.shadow.top;
    frame->max_shadow_extents.bottom = borders.shadow.bottom;
    frame->max_shadow_extents.left = borders.shadow.left;
    frame->max_shadow_extents.right = borders.shadow.right;

    gwd_decor_frame_unref (frame);
}

static void
gwd_theme_metacity_get_event_window_position (GWDTheme *theme,
                                              decor_t  *decor,
                                              gint      i,
                                              gint      j,
                                              gint      width,
                                              gint      height,
                                              gint     *x,
                                              gint     *y,
                                              gint     *w,
                                              gint     *h)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);
    MetaFrameGeometry fgeom;
    MetaFrameFlags flags;
    GtkBorder visible;
    GtkBorder resize;
    GtkBorder total;
    gint top_border;

    get_decoration_geometry (metacity, decor, &flags, &fgeom,
                             frame_type_from_string (decor->frame->type));

    visible = fgeom.borders.visible;
    resize = fgeom.borders.resize;

    /* We can not use `fgeom->borders.total` border here - it includes also
     * `shadow` border, but it is not included in frame extents! Create new
     * `total` border that includes only `visible` border and `resize` border.
     */
    total.left = visible.left + resize.left;
    total.right = visible.right + resize.right;
    total.top = visible.top + resize.top;
    total.bottom = visible.bottom + resize.bottom;

    width += total.left + total.right;
    height += total.top + total.bottom;

    top_border = fgeom.title_rect.y - fgeom.borders.invisible.top;

#define TOP_RESIZE_HEIGHT 2
#define RESIZE_EXTENDS 15

    switch (i) {
        case 2: /* bottom */
            switch (j) {
                case 2: /* bottom right */
                    *x = width - total.right - RESIZE_EXTENDS;
                    *y = height - total.bottom - RESIZE_EXTENDS;

                    *w = total.right + RESIZE_EXTENDS;
                    *h = total.bottom + RESIZE_EXTENDS;
                    break;
                case 1: /* bottom */
                    *x = total.left + RESIZE_EXTENDS;
                    *y = height - total.bottom;

                    *w = width - total.left - total.right - (2 * RESIZE_EXTENDS);
                    *h = total.bottom;
                    break;
                case 0: /* bottom left */
                default:
                    *x = 0;
                    *y = height - total.bottom - RESIZE_EXTENDS;

                    *w = total.left + RESIZE_EXTENDS;
                    *h = total.bottom + RESIZE_EXTENDS;
                    break;
            }
            break;
        case 1: /* middle */
            switch (j) {
                case 2: /* right */
                    *x = width - total.right;
                    *y = resize.top + top_border + RESIZE_EXTENDS;

                    *w = total.right;
                    *h = height - resize.top - top_border - total.bottom - (2 * RESIZE_EXTENDS);
                    break;
                case 1: /* middle */
                    *x = total.left;
                    *y = resize.top + top_border + TOP_RESIZE_HEIGHT;
                    *w = width - total.left - total.right;
                    *h = visible.top - top_border - TOP_RESIZE_HEIGHT;
                    break;
                case 0: /* left */
                default:
                    *x = 0;
                    *y = resize.top + top_border + RESIZE_EXTENDS;

                    *w = total.left;
                    *h = height - resize.top - top_border - total.bottom - (2 * RESIZE_EXTENDS);
                    break;
            }
            break;
        case 0: /* top */
        default:
            switch (j) {
                case 2: /* top right */
                    *x = width - total.right - RESIZE_EXTENDS;
                    *y = 0;

                    *w = total.right + RESIZE_EXTENDS;
                    *h = resize.top + top_border + RESIZE_EXTENDS;
                    break;
                case 1: /* top */
                    *x = total.left + RESIZE_EXTENDS;
                    *y = 0;

                    *w = width - total.left - total.right - (2 * RESIZE_EXTENDS);
                    *h = resize.top + top_border + TOP_RESIZE_HEIGHT;
                    break;
                case 0: /* top left */
                default:
                    *x = 0;
                    *y = 0;

                    *w = total.left + RESIZE_EXTENDS;
                    *h = resize.top + top_border + RESIZE_EXTENDS;
                    break;
            }
            break;
    }

    if (!(flags & META_FRAME_ALLOWS_VERTICAL_RESIZE)) {
        /* turn off top and bottom event windows */
        if (i == 0 || i == 2)
            *w = *h = 0;
    }

    if (!(flags & META_FRAME_ALLOWS_HORIZONTAL_RESIZE)) {
        /* turn off left and right event windows */
        if (j == 0 || j == 2)
            *w = *h = 0;
    }

#undef TOP_RESIZE_HEIGHT
#undef RESIZE_EXTENDS
}

static gboolean
gwd_theme_metacity_get_button_position (GWDTheme *theme,
                                        decor_t  *decor,
                                        gint      i,
                                        gint      width,
                                        gint      height,
                                        gint     *x,
                                        gint     *y,
                                        gint     *w,
                                        gint     *h)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);
    MetaFrameGeometry fgeom;
    MetaFrameType frame_type;
    MetaFrameFlags flags;

    if (!decor->context) {
        /* undecorated windows implicitly have no buttons */
        return FALSE;
    }

    frame_type = frame_type_from_string (decor->frame->type);

    get_decoration_geometry (metacity, decor, &flags, &fgeom, frame_type);

    MetaButtonType button_type = button_type_to_meta_button_type (i);
    MetaButton **buttons = meta_theme_get_buttons (metacity->theme);

    for (gint index = 0; buttons[index]; index++) {
        if (meta_button_get_type (buttons[index]) == button_type) {
            GdkRectangle rect;

            meta_button_get_event_rect (buttons[index], &rect);

            if (rect.width != 0 && rect.height != 0) {
                *x = rect.x;
                *y = rect.y;
                *w = rect.width;
                *h = rect.height;

                *x = *x - fgeom.borders.invisible.left + fgeom.borders.resize.left;
                *y = *y - fgeom.borders.invisible.top + fgeom.borders.resize.top;

                g_free (buttons);
                return TRUE;
            }
        }
    }

    g_free (buttons);
    return FALSE;
}

static void
gwd_theme_metacity_update_titlebar_font (GWDTheme                   *theme,
                                         const PangoFontDescription *titlebar_font)
{
    GWDThemeMetacity *metacity = GWD_THEME_METACITY (theme);

    meta_theme_set_titlebar_font (metacity->theme, titlebar_font);
}

static void
gwd_theme_metacity_class_init (GWDThemeMetacityClass *metacity_class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (metacity_class);
    GWDThemeClass *theme_class = GWD_THEME_CLASS (metacity_class);

    object_class->constructed = gwd_theme_metacity_constructed;
    object_class->dispose = gwd_theme_metacity_dispose;

    theme_class->dpi_changed = gwd_theme_metacity_dpi_changed;
    theme_class->scale_changed = gwd_theme_metacity_scale_changed;
    theme_class->style_updated = gwd_theme_metacity_style_updated;
    theme_class->draw_window_decoration = gwd_theme_metacity_draw_window_decoration;
    theme_class->calc_decoration_size = gwd_theme_metacity_calc_decoration_size;
    theme_class->update_border_extents = gwd_theme_metacity_update_border_extents;
    theme_class->get_event_window_position = gwd_theme_metacity_get_event_window_position;
    theme_class->get_button_position = gwd_theme_metacity_get_button_position;
    theme_class->update_titlebar_font = gwd_theme_metacity_update_titlebar_font;
}

static void
gwd_theme_metacity_init (GWDThemeMetacity *metacity)
{
}

/**
 * gwd_theme_metacity_new:
 * @settings: a #GWDSettings
 *
 * Creates a new #GWDTheme. If meta_theme_load will fail to load Metacity
 * theme then this function will return %NULL. In this case #GWDThemeCairo
 * must be used as fallback.
 *
 * This function MUST be used only in gwd_theme_new!
 *
 * Returns: (transfer full) (nullable): a newly created #GWDTheme, or %NULL
 */
GWDTheme *
gwd_theme_metacity_new (GWDSettings *settings)
{
    GWDThemeMetacity *metacity;

    metacity = g_object_new (GWD_TYPE_THEME_METACITY,
                             "settings", settings,
                             NULL);

    /* We failed to load Metacity theme */
    if (metacity->theme == NULL) {
        g_object_unref (metacity);
        return NULL;
    }

    return GWD_THEME (metacity);
}
