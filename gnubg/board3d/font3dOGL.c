
#include "config.h"
#include "legacyGLinc.h"
#include "fun3d.h"
#include "util.h"
#include "output.h"
#include <glib/gi18n.h>

#include <ft2build.h>
#include FT_FREETYPE_H

extern void PopulateVectoriser(Vectoriser* pVect, const FT_Outline* pOutline);
extern void TidyMemory(const Vectoriser* pVect, const Mesh* pMesh);

extern void TESS_CALLBACK tcbError(GLenum error);
extern void TESS_CALLBACK tcbVertex(void* data, Mesh* UNUSED(pMesh));
extern void TESS_CALLBACK tcbCombine(const double coords[3], const double* UNUSED(vertex_data[4]), float UNUSED(weight[4]), void** outData);
extern void TESS_CALLBACK tcbBegin(GLenum type, Mesh* UNUSED(pMesh));
extern void TESS_CALLBACK tcbEnd( /*lint -e{818} */ Mesh* pMesh);

int RenderGlyph(const FT_Outline* pOutline, int AA);
void PopulateMesh(const Vectoriser* pVect, Mesh* pMesh);
extern int RenderGlyphX(const FT_Outline* pOutline);

extern void
RenderString3d(const OGLFont* pFont, const char* str, float scale, int MAA)
{
	glScalef(scale, scale * pFont->heightRatio, 1.f);

	while (*str) {
		int offset = *str - '0';
		if (!MAA)
		{
			/* Draw character */
			OglModelDraw(&pFont->models[offset]);
		}
		else
		{
			/* AA outline of character */
			glCallList(pFont->AAglyphs + (unsigned int)offset);
		}
		/* Move on to next place */
		glTranslatef((float)(pFont->advance + GetKern(pFont, str[0], str[1])) / 64.0f, 0.f, 0.f);

		str++;
	}
}

int
RenderText(const char* text, FT_Library ftLib, OGLFont* pFont, const char* pPath, int pointSize, float scale,
	float heightRatio)
{
	int len = 0;
	FT_Face face;

	if (FT_New_Face(ftLib, pPath, 0, &face))
		return 0;

	if (FT_Set_Char_Size(face, 0, pointSize * 64 /* 26.6 fractional points */, 0, 0))
		return 0;

	/* Create text glyphs */
	pFont->textGlyphs = glGenLists(1);
	glNewList(pFont->textGlyphs, GL_COMPILE);

	pFont->scale = scale;
	pFont->height = 0;

	while (*text) {
		/* Draw character */
		FT_ULong charCode = (FT_ULong)(int)(*text);
		unsigned int glyphIndex = FT_Get_Char_Index(face, charCode);
		if (!glyphIndex)
			return 0;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_HINTING))
			return 0;
		g_assert(face->glyph->format == ft_glyph_format_outline);
		if (pFont->height == 0)
			pFont->height = (float)face->glyph->metrics.height * scale * heightRatio / 64;


		if (!RenderGlyph(&face->glyph->outline, 0))
			return 0;

		text++;
		len += face->glyph->advance.x;
		/* Move on to next place */
		if (*text) {
			unsigned int nextGlyphIndex;
			int kern = 0;
			FT_Vector kernAdvance;
			charCode = (FT_ULong)(int)(*text);
			nextGlyphIndex = FT_Get_Char_Index(face, charCode);
			if (nextGlyphIndex && !FT_Get_Kerning(face, glyphIndex, nextGlyphIndex, ft_kerning_unfitted, &kernAdvance))
				kern = kernAdvance.x;

			glTranslatef((face->glyph->advance.x + kern) / 64.0f, 0.f, 0.f);
			len += kern;
		}
	}
	glEndList();

	pFont->length = (len / 64.0f) * pFont->scale;

	return !FT_Done_Face(face);
}

int
CreateFontText(OGLFont* ppFont, const char* text, const char* fontFile, int pitch, float size, float heightRatio)
{
	char* filename;

	FT_Library ftLib;
	if (FT_Init_FreeType(&ftLib))
		return 0;

	filename = BuildFilename(fontFile);
	if (!RenderText(text, ftLib, ppFont, filename, pitch, size, heightRatio)) {
		outputerrf(_("Failed to create font from (%s)\n"), filename);
		g_free(filename);
		return 0;
	}
	g_free(filename);
	return !FT_Done_FreeType(ftLib);
}

int
CreateOGLFont(FT_Library ftLib, OGLFont* pFont, const char* pPath, int pointSize, float scale, float heightRatio)
{
	unsigned int i, j;
	FT_Face face;

	memset(pFont, 0, sizeof(OGLFont));
	pFont->scale = scale;
	pFont->heightRatio = heightRatio;

	if (FT_New_Face(ftLib, pPath, 0, &face))
		return 0;

	if (FT_Set_Char_Size(face, 0, pointSize * 64 /* 26.6 fractional points */, 0, 0))
		return 0;

	{                           /* Find height of "1" */
		unsigned int glyphIndex = FT_Get_Char_Index(face, '1');
		if (!glyphIndex)
			return 0;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_HINTING) || (face->glyph->format != ft_glyph_format_outline))
			return 0;

		pFont->height = (float)face->glyph->metrics.height * scale * heightRatio / 64;
		pFont->advance = face->glyph->advance.x;
	}

	/* Create digit glyphs */
	pFont->AAglyphs = glGenLists(10);

	for (i = 0; i < 10; i++) {
		unsigned int glyphIndex = FT_Get_Char_Index(face, '0' + i);
		if (!glyphIndex)
			return 0;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_NO_HINTING))
			return 0;
		g_assert(face->glyph->format == ft_glyph_format_outline);

		CALL_OGL(pFont->models[i], RenderGlyphX, &face->glyph->outline);

		glNewList(pFont->AAglyphs + i, GL_COMPILE);
		if (!RenderGlyph(&face->glyph->outline, 1))
			return 0;
		glEndList();

		/* Calculate kerning table */
		for (j = 0; j < 10; j++) {
			FT_Vector kernAdvance;
			unsigned int nextGlyphIndex = FT_Get_Char_Index(face, '0' + j);
			if (!nextGlyphIndex || FT_Get_Kerning(face, glyphIndex, nextGlyphIndex, ft_kerning_unfitted, &kernAdvance))
				return 0;
			pFont->kern[i][j] = kernAdvance.x;
		}
	}

	return !FT_Done_Face(face);
}

int
RenderGlyph(const FT_Outline* pOutline, int AA)
{
	Mesh mesh;
	unsigned int index, point;

	Vectoriser vect;
	PopulateVectoriser(&vect, pOutline);
	if ((vect.contours->len < 1) || (vect.numPoints < 3))
		return 0;

	/* Solid font */
	PopulateMesh(&vect, &mesh);

	if (!AA)
	{	// Inner part of font
		for (index = 0; index < mesh.tesselations->len; index++) {
			Tesselation* subMesh = &g_array_index(mesh.tesselations, Tesselation, index);

			glBegin(subMesh->meshType);
			for (point = 0; point < subMesh->tessPoints->len; point++) {
				Point3d* pPoint = &g_array_index(subMesh->tessPoints, Point3d, point);

				g_assert(pPoint->data[2] == 0);
				glVertex2f((float)pPoint->data[0] / 64.0f, (float)pPoint->data[1] / 64.0f);
			}
			glEnd();
		}
	}
	else
	{	/* Outline font - used to anti-alias edges */
		for (index = 0; index < vect.contours->len; index++) {
			Contour* contour = &g_array_index(vect.contours, Contour, index);

			glBegin(GL_LINE_LOOP);
			for (point = 0; point < contour->conPoints->len; point++) {
				Point3d* pPoint = &g_array_index(contour->conPoints, Point3d, point);
				glVertex2f((float)pPoint->data[0] / 64.0f, (float)pPoint->data[1] / 64.0f);
			}
			glEnd();
		}
	}
	TidyMemory(&vect, &mesh);

	return 1;
}

void
FreeNumberFont(OGLFont* ppFont)
{
	if (ppFont->AAglyphs != 0) {
		glDeleteLists(ppFont->AAglyphs, 10);
		ppFont->AAglyphs = 0;
	}
}

void
FreeTextFont(OGLFont* ppFont)
{
	if (ppFont->textGlyphs != 0) {
		glDeleteLists(ppFont->textGlyphs, 1);
		ppFont->textGlyphs = 0;
	}
}

#if defined(USE_APPLE_OPENGL)
#define GLUFUN(X) X
#elif defined(__GNUC__)
#if defined(WIN32)
typedef APIENTRY GLvoid(*_GLUfuncptr) ();
#endif
#define GLUFUN(X) (_GLUfuncptr)X
#else
#define GLUFUN(X) X
#endif

extern GList* combineList;

void
PopulateMesh(const Vectoriser* pVect, Mesh* pMesh)
{
	unsigned int c, p;
	GLUtesselator* tobj = gluNewTess();

	combineList = NULL;

	pMesh->tesselations = g_array_new(FALSE, FALSE, sizeof(Tesselation));

	gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, GLUFUN(tcbBegin));
	gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, GLUFUN(tcbVertex));
	gluTessCallback(tobj, GLU_TESS_COMBINE, GLUFUN(tcbCombine));
	gluTessCallback(tobj, GLU_TESS_END_DATA, GLUFUN(tcbEnd));
	gluTessCallback(tobj, GLU_TESS_ERROR, GLUFUN(tcbError));

	gluTessProperty(tobj, GLU_TESS_WINDING_RULE, (double)GLU_TESS_WINDING_ODD);

	gluTessNormal(tobj, 0.0, 0.0, 1.0);

	gluTessBeginPolygon(tobj, pMesh);

	for (c = 0; c < pVect->contours->len; ++c) {
		Contour* contour = &g_array_index(pVect->contours, Contour, c);

		gluTessBeginContour(tobj);

		for (p = 0; p < contour->conPoints->len; p++) {
			Point3d* point = &g_array_index(contour->conPoints, Point3d, p);
			gluTessVertex(tobj, point->data, point->data);
		}

		gluTessEndContour(tobj);
	}

	gluTessEndPolygon(tobj);

	gluDeleteTess(tobj);
}

extern void
glPrintCube(const OGLFont* cubeFont, const char* text, int MAA)
{
	/* Align horizontally and vertically */
	float scale = cubeFont->scale;
	float heightOffset = cubeFont->height;
	if (strlen(text) > 1) {     /* Make font smaller for 2 digit cube numbers */
		scale *= CUBE_TWODIGIT_FACTOR;
		heightOffset *= CUBE_TWODIGIT_FACTOR;
	}
	glTranslatef(-getTextLen3d(cubeFont, text) / 2.0f, -heightOffset / 2.0f, 0.f);
	RenderString3d(cubeFont, text, scale, MAA);
}

extern void
glPrintNumbersRA(const OGLFont* numberFont, const char* text)
{
	/* Right align */
	glTranslatef(-getTextLen3d(numberFont, text), 0.f, 0.f);
	RenderString3d(numberFont, text, numberFont->scale, 0);
}

extern void
glDrawText(const OGLFont* font)
{
	glTranslatef(-font->length / 2.f, -font->height / 2.f, 0.f);
	glScalef(font->scale, font->scale, 1.f);
	glCallList(font->textGlyphs);
}
