
#include "config.h"
#include "inc3d.h"

#if _MSC_VER && !TEST_HARNESS
// Not yet working...
GtkWidget* GLWidgetCreate(RealizeCB realizeCB, ConfigureCB configureCB, ExposeCB exposeCB, void* data)
{ return NULL; }
void GLWidgetMakeCurrent(GtkWidget* widget)
{}
gboolean GLWidgetRender(GtkWidget* widget, ExposeCB exposeCB, GdkEventExpose* eventDetails, void* data)
{ return 0; }
gboolean GLInit(int* argc, char*** argv)
{ return 0; }

#else

#include <gtk/gtkgl.h>

void GLWidgetMakeCurrent(GtkWidget* widget)
{
	if (!gdk_gl_drawable_make_current(gtk_widget_get_gl_drawable(widget), gtk_widget_get_gl_context(widget)))
		g_print("gdk_gl_drawable_make_current failed!\n");
}

extern GdkGLConfig*
getGlConfig(void)
{
	static GdkGLConfig* glconfig = NULL;
	if (!glconfig)
		glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_STENCIL);
	if (!glconfig) {
		glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
		g_warning("Stencil buffer not available, no shadows\n");
	}
	if (!glconfig) {
		g_warning("*** No appropriate OpenGL-capable visual found.\n");
	}
	return glconfig;
}

typedef struct _GLWidgetData
{
	RealizeCB realizeCB;
	ConfigureCB configureCB;
	ExposeCB exposeCB;
	void* cbData;
} GLWidgetData;

static void
realize_event(GtkWidget* widget, const GLWidgetData* glwData)
{
	GdkGLDrawable* gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return;

	glwData->realizeCB(glwData->cbData);

	gdk_gl_drawable_gl_end(gldrawable);
}

static gboolean
configure_event(GtkWidget* widget, GdkEventConfigure* UNUSED(eventDetails), const GLWidgetData* glwData)
{
	GdkGLDrawable* gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return FALSE;

	glwData->configureCB(widget, glwData->cbData);

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

gboolean GLWidgetRender(GtkWidget* widget, ExposeCB exposeCB, GdkEventExpose* eventDetails, void* data)
{
	GdkGLDrawable* gldrawable = gtk_widget_get_gl_drawable(widget);

	if (!gdk_gl_drawable_gl_begin(gldrawable, gtk_widget_get_gl_context(widget)))
		return TRUE;
	CheckOpenglError();

	if (exposeCB(widget, eventDetails, data))
		gdk_gl_drawable_swap_buffers(gldrawable);

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

static gboolean
expose_event(GtkWidget* widget, GdkEventExpose* eventDetails, const GLWidgetData* glwData)
{
	return GLWidgetRender(widget, glwData->exposeCB, eventDetails, glwData->cbData);
}

GtkWidget*
GLWidgetCreate(RealizeCB realizeCB, ConfigureCB configureCB, ExposeCB exposeCB, void *data)
{
	GtkWidget* pw = gtk_drawing_area_new();

	if (pw == NULL) {
		g_print("Can't create opengl drawing widget\n");
		return NULL;
	}

	/* Set OpenGL-capability to the widget - no list sharing */
	if (!gtk_widget_set_gl_capability(pw, getGlConfig(), NULL, TRUE, GDK_GL_RGBA_TYPE))
	{
		g_print("Can't create opengl capable widget\n");
		return NULL;
	}

	GLWidgetData* glwData = malloc(sizeof(GLWidgetData));
	glwData->cbData = data;
	glwData->realizeCB = realizeCB;
	glwData->configureCB = configureCB;
	glwData->exposeCB = exposeCB;

	if (realize_event != NULL)
		g_signal_connect(G_OBJECT(pw), "realize", G_CALLBACK(realize_event), glwData);
	if (configure_event != NULL)
		g_signal_connect(G_OBJECT(pw), "configure_event", G_CALLBACK(configure_event), glwData);
	if (expose_event != NULL)
		g_signal_connect(G_OBJECT(pw), "expose_event", G_CALLBACK(expose_event), glwData);

	return pw;
}

gboolean GLInit(int* argc, char*** argv)
{
	return gtk_gl_init_check(argc, argv);
}

#endif
