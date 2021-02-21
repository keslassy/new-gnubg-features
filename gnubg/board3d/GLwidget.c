/*
 * Copyright (C) 2019-2021 Jon Kinsey <jonkinsey@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * $Id$
 */

#include "config.h"
#ifdef USE_GTK3
#include <epoxy/gl.h>
#endif
#include "fun3d.h"
#include "util.h"
#include "common.h"

#include <cglm/affine.h>

typedef struct _GLWidgetData
{
	RealizeCB realizeCB;
	ConfigureCB configureCB;
	ExposeCB exposeCB;
	void* cbData;
} GLWidgetData;

#ifdef USE_GTK3

guint basicShader;
guint projection_location;
guint modelView_location;
guint colour_location;

void
setMaterial(const Material* pMat)
{
	if (pMat != NULL && pMat != currentMat)
	{
		currentMat = pMat;
		glUniform3fv(colour_location, 1, pMat->diffuseColour);
	}
}

void
setMaterialReset(const Material* pMat)
{
	currentMat = NULL;
	setMaterial(pMat);
}

void ModelManagerCopyModelToBuffer(ModelManager* modelHolder, int modelNumber)
{
	glBufferSubData(GL_ARRAY_BUFFER, modelHolder->models[modelNumber].dataStart * sizeof(float), modelHolder->models[modelNumber].dataLength * sizeof(float), modelHolder->models[modelNumber].data);
	free(modelHolder->models[modelNumber].data);
	modelHolder->models[modelNumber].data = NULL;
}

void ModelManagerCreate(ModelManager* modelHolder)
{
	int model;
	if (modelHolder->vao == GL_INVALID_VALUE)
	{
		/* we need to create a VAO to store the other buffers */
		glGenVertexArrays(1, &modelHolder->vao);
		glBindVertexArray(modelHolder->vao);

		/* this is the VBO that holds the vertex data */
		glGenBuffers(1, &modelHolder->buffer);
		glBindBuffer(GL_ARRAY_BUFFER, modelHolder->buffer);

		/* get the location of the "position" and "color" attributes */
		guint texCoord_index = glGetAttribLocation(basicShader, "texCoordAttrib");
		guint normal_index = glGetAttribLocation(basicShader, "normalAttrib");
		guint position_index = glGetAttribLocation(basicShader, "positionAttrib");

		int stride = VERTEX_STRIDE * sizeof(float);
		/* enable and set the attributes */
		glEnableVertexAttribArray(texCoord_index);
		glVertexAttribPointer(texCoord_index, 2, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(normal_index);
		glVertexAttribPointer(normal_index, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(2 * sizeof(float)));
		glEnableVertexAttribArray(position_index);
		glVertexAttribPointer(position_index, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(5 * sizeof(float)));
	}
	if (modelHolder->totalNumVertices > modelHolder->allocNumVertices)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * modelHolder->totalNumVertices, NULL, GL_STATIC_DRAW);
		modelHolder->allocNumVertices = modelHolder->totalNumVertices;
	}

	/* Copy data into gpu buffer */
	int vertexPos = 0;
	for (model = 0; model < modelHolder->numModels; model++)
	{
		modelHolder->models[model].dataStart = vertexPos;
		ModelManagerCopyModelToBuffer(modelHolder, model);
		vertexPos += modelHolder->models[model].dataLength;
	}
	g_assert(vertexPos == modelHolder->totalNumVertices);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GLWidgetMakeCurrent(GtkWidget* widget)
{
	//TODO: gtk_gl_area_make_current "not usually needed to be called"?
	gtk_gl_area_make_current(GTK_GL_AREA(widget));
}

char* LoadFile(const char* filename)
{
	long lSize;
	char* buffer;

	FILE* fp = fopen(filename, "rb");
	if (!fp)
	{
		printf("Failed to open %s!\n", filename);
		return NULL;
	}

	fseek(fp, 0L, SEEK_END);
	lSize = ftell(fp);
	if (lSize == -1) {
                fclose(fp);
                return NULL;
        }

	rewind(fp);

	/* allocate memory for entire content */
	buffer = calloc(1, lSize + 1);
	if (!buffer) {
                fclose(fp);
                return NULL;
        }

	fread(buffer, lSize, 1, fp);

	fclose(fp);
	return buffer;
}

guint CreateShader(int shader_type, const char* shader_name)
{
	int status;
	char* source;
	char* pathname = BuildFilename(shader_name);
	char* filename;

	if (shader_type == GL_VERTEX_SHADER)
		filename = g_strdup_printf("%s-vertex.glsl", pathname);
	else
		filename = g_strdup_printf("%s-fragment.glsl", pathname);
	g_free(pathname);

	source = LoadFile(filename);
	g_free(filename);

	guint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		int log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

		char* buffer = g_malloc(log_len + 1);
		glGetShaderInfoLog(shader, log_len, NULL, buffer);
		printf("Compilation failure in %s shader: %s", shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment", buffer);
		g_free(buffer);
		return 0;
	}

	free(source);
	return shader;
}

guint
init_shaders(const char* shader_name)
{
	guint vertex = CreateShader(GL_VERTEX_SHADER, shader_name);
	guint fragment = CreateShader(GL_FRAGMENT_SHADER, shader_name);

	/* link the vertex and fragment shaders together */
	guint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	int status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		int log_len = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

		char* buffer = g_malloc(log_len + 1);
		glGetProgramInfoLog(program, log_len, NULL, buffer);

		printf("Linking failure in program: %s", buffer);

		g_free(buffer);
		return FALSE;
	}

	return program;
}

static void
realize_event(GtkWidget* widget, const GLWidgetData* glwData)
{
	GLWidgetMakeCurrent(widget);
	if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
		return;
	gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(widget), TRUE);

	/* initialize the shaders and retrieve the program data */
	basicShader = init_shaders("/Shaders/basic");
	/* get the location of the matrices uniforms */
	projection_location = glGetUniformLocation(basicShader, "projection");
	modelView_location = glGetUniformLocation(basicShader, "modelView");
	colour_location = glGetUniformLocation(basicShader, "colour");
	glwData->realizeCB(glwData->cbData);

	gtk_widget_queue_draw(widget);
}

void
resize_event(GtkGLArea* widget, gint width, gint height, const GLWidgetData* glwData)
{
	glwData->configureCB(GTK_WIDGET(widget), glwData->cbData);
}

void OglModelDraw(const ModelManager* modelManager, int modelNumber, const Material* pMat)
{
	setMaterial(pMat);

	/* update the projection matrices we use in the shader */
	glUniformMatrix4fv(projection_location, 1, GL_FALSE, GetProjectionMatrix());
	glUniformMatrix4fv(modelView_location, 1, GL_FALSE, GetModelViewMatrix());

	/* use the buffers in the VAO */
	glBindVertexArray(modelManager->vao);

	/* draw the vertices in the model */
	glDrawArrays(GL_TRIANGLES, modelManager->models[modelNumber].dataStart / VERTEX_STRIDE, modelManager->models[modelNumber].dataLength / VERTEX_STRIDE);
}

void OglBindBuffer(ModelManager* modelHolder)
{
	/* this is the VBO that holds the vertex data */
	glBindBuffer(GL_ARRAY_BUFFER, modelHolder->buffer);
}

gboolean GLWidgetRender(GtkWidget* widget, ExposeCB exposeCB, GdkEventExpose* eventDetails, void* data)
{
	CheckOpenglError();

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(basicShader);

	exposeCB(widget, eventDetails, data);

	return TRUE;
}

static gboolean
expose_event(GtkWidget* widget, GdkEventExpose* eventDetails, const GLWidgetData* glwData)
{
	return GLWidgetRender(widget, glwData->exposeCB, eventDetails, glwData->cbData);
}

GtkWidget* GLWidgetCreate(RealizeCB realizeCB, ConfigureCB configureCB, ExposeCB exposeCB, void* data)
{
	GtkWidget* pw = gtk_gl_area_new();

	if (pw == NULL) {
		g_print("Can't create opengl drawing widget\n");
		return NULL;
	}

	GLWidgetData* glwData = g_malloc(sizeof(GLWidgetData));
	if (!glwData) return NULL;

	glwData->cbData = data;
	glwData->realizeCB = realizeCB;
	glwData->configureCB = configureCB;
	glwData->exposeCB = exposeCB;

	if (realizeCB != NULL)
		g_signal_connect(G_OBJECT(pw), "realize", G_CALLBACK(realize_event), glwData);
	if (configureCB != NULL)
		g_signal_connect(G_OBJECT(pw), "resize", G_CALLBACK(resize_event), glwData);
	if (exposeCB != NULL)
		g_signal_connect(G_OBJECT(pw), "render", G_CALLBACK(expose_event), glwData);

	g_object_set_data_full(G_OBJECT(pw), "GLWidgetData", glwData, g_free);

	return pw;
}

gboolean GLInit(int* argc, char*** argv)
{
	return GL_TRUE;	// TODO: Anything to check here?
}

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
configure_event(GtkWidget* widget, GdkEventConfigure* UNUSED(eventDetails), void* data)
{
	const GLWidgetData* glwData = (const GLWidgetData*)data;
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

	GLWidgetData* glwData = g_malloc(sizeof(GLWidgetData));
	glwData->cbData = data;
	glwData->realizeCB = realizeCB;
	glwData->configureCB = configureCB;
	glwData->exposeCB = exposeCB;

	if (realizeCB != NULL)
		g_signal_connect(G_OBJECT(pw), "realize", G_CALLBACK(realize_event), glwData);
	if (configureCB != NULL)
		g_signal_connect(G_OBJECT(pw), "configure_event", G_CALLBACK(configure_event), glwData);
	if (exposeCB != NULL)
		g_signal_connect(G_OBJECT(pw), "expose_event", G_CALLBACK(expose_event), glwData);

	g_object_set_data_full(G_OBJECT(pw), "GLWidgetData", glwData, g_free);

	return pw;
}

gboolean GLInit(int* argc, char*** argv)
{
	return gtk_gl_init_check(argc, argv);
}

#endif
