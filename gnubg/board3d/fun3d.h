/*
 * Copyright (C) 2006-2019 Jon Kinsey <jonkinsey@gmail.com>
 * Copyright (C) 2007-2016 the AUTHORS
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

#ifndef FUN3D_H
#define FUN3D_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "common.h"
#include "inc3d.h"
#include "matrix.h"
#include "gtkwindows.h"

#include "model.h"

extern int fClockwise;          /* Player 1 moves clockwise */

extern const Material* currentMat;

#ifndef TYPES3D_H
#define TYPES3D_H

typedef struct _Path Path;
typedef struct _OGLFont OGLFont;
typedef float GLfloat;
typedef unsigned int GLenum;

struct _Texture {
	unsigned int texID;
	int width;
	int height;
};

typedef struct _OglModel
{
	float* data;
	int dataLength;
	int dataStart;
} OglModel;

enum ModelType { MT_BACKGROUND, MT_BASE, MT_ODDPOINTS, MT_EVENPOINTS, MT_TABLE, MT_HINGE, MT_CUBE, MT_MOVEINDICATOR, MT_PIECE, MT_PIECETOP, MT_DICE, MT_FLAG };

#define MAX_MODELS 12
typedef struct _ModelManager
{
	int totalNumVertices;
	int allocNumVertices;
	float* vertexData;
	int numModels;
	OglModel models[MAX_MODELS];

#ifdef USE_GTK3
	guint vao;
	guint buffer;
#endif
} ModelManager;

void OglModelInit(ModelManager* modelHolder, int modelNumber);
void OglModelAlloc(ModelManager* modelHolder, int modelNumber);
void OglModelDraw(const ModelManager* modelManager, int modelNumber, const Material* pMat);

void ModelManagerInit(ModelManager* modelHolder);
void ModelManagerStart(ModelManager* modelHolder);
void ModelManagerCreate(ModelManager* modelHolder);
void ModelManagerCopyModelToBuffer(ModelManager* modelHolder, int modelNumber);

#define VERTEX_STRIDE 8

/* TODO: Tidy this up... (i.e. remove tricky macro) */
extern OglModel* curModel;
#define CALL_OGL(modelManager, modelNumber, fun, ...) \
{ \
	curModel = modelManager.models[modelNumber];\
	OglModelInit(modelManager, modelNumber);\
	fun(__VA_ARGS__);\
	OglModelAlloc(modelManager, modelNumber);\
	fun(__VA_ARGS__); \
}

#define F_PI (float)G_PI
#define F_PI_2 (float)G_PI_2

typedef signed long  FT_Pos;

struct _OGLFont {
	unsigned int textGlyphs;	// Used in graph
	unsigned int AAglyphs;	// For Anti-aliasing
	FT_Pos advance;
	FT_Pos kern[10][10];
	float scale;
	float heightRatio;
	float height;
	float length;
	ModelManager modelManager;
};

typedef struct _Point3d {
	double data[3];
} Point3d;

#if defined(WIN32)
/* Need to set the callback calling convention for windows */
#define TESS_CALLBACK __stdcall
#else
#define TESS_CALLBACK
#endif

typedef struct _GArray		GArray;
typedef unsigned char GLboolean;

typedef struct _Contour {
	GArray* conPoints;
} Contour;

typedef struct _Vectoriser {
	GArray* contours;
	unsigned int numPoints;
} Vectoriser;

typedef struct _Tesselation {
	GArray* tessPoints;
	GLenum meshType;
} Tesselation;

typedef struct _Mesh {
	GArray* tesselations;
} Mesh;

#define TT_COUNT 3              /* 3 texture types: general, piece and hinge */

typedef int idleFunc(BoardData3d* bd3d);

/* Work out which sides of dice to draw */
typedef struct _diceTest {
	int top, bottom, side[4];
} diceTest;

typedef struct _DiceRotation {
	float xRotStart, yRotStart;
	float xRot, yRot;
	float xRotFactor, yRotFactor;
} DiceRotation;

typedef struct _EigthPoints
{	/* Used for rounded corners */
	float*** points;
	unsigned int accuracy;
} EigthPoints;

extern void freeEigthPoints(EigthPoints* eigthPoints);
void calculateEigthPoints(EigthPoints* eigthPoints, float radius, unsigned int accuracy);

/* Used to calculate correct viewarea for board/fov angles */
typedef struct _viewArea {
	float top;
	float bottom;
	float width;
} viewArea;

//TODO: Sort out (remove gluNurb?)
typedef struct GLUnurbs GLUnurbsObj;
typedef struct _Flag3d {
	/* Define nurbs surface - for flag */
	GLUnurbsObj* flagNurb;
#define S_NUMPOINTS 4
#define S_NUMKNOTS (S_NUMPOINTS * 2)
#define T_NUMPOINTS 2
#define T_NUMKNOTS (T_NUMPOINTS * 2)
	/* Control points for the flag. The Z values are modified to make it wave */
	float ctlpoints[S_NUMPOINTS][T_NUMPOINTS][3];
} Flag3d;

/* Font info */

#define FONT_PITCH 24
#define FONT_SIZE (base_unit / 20.0f)
#define FONT_HEIGHT_RATIO 1.f

#define CUBE_FONT_PITCH 34
#define CUBE_FONT_SIZE (base_unit / 24.0f)
#define CUBE_FONT_HEIGHT_RATIO 1.25f

#define FONT_VERA "fonts/Vera.ttf"
#define FONT_VERA_SERIF_BOLD "fonts/VeraSeBd.ttf"

#endif

/* Setup functions */
void InitGL(const BoardData* bd);

/* Drawing functions */
void drawBoard(const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd);
extern void Draw3d(const BoardData* bd);
float getBoardWidth(void);
float getBoardHeight(void);
void calculateBackgroundSize(BoardData3d* bd3d, const int viewport[4]);

extern void getPiecePos(unsigned int point, unsigned int pos, float v[3]);

/* Misc functions */
void SetupSimpleMat(Material* pMat, float r, float g, float b);
void SetupMat(Material* pMat, float r, float g, float b, float dr, float dg, float db, float sr, float sg, float sb,
	int shin, float a);
void setMaterial(const Material* pMat);
void SetColour3d(float r, float g, float b, float a);
float randRange(float range);
void setupPath(const BoardData* bd, Path* p, float* pRotate, unsigned int fromPoint, unsigned int fromDepth,
	unsigned int toPoint, unsigned int toDepth);
int finishedPath(const Path* p);
void getProjectedPieceDragPos(int x, int y, float pos[3]);
void updateMovingPieceOccPos(const BoardData* bd, BoardData3d* bd3d);
void LoadTextureInfo(void);
GList* GetTextureList(TextureType type);
extern void FindNamedTexture(TextureInfo** textureInfo, char* name);
float Dist2d(float a, float b);
float*** Alloc3d(unsigned int x, unsigned int y, unsigned int z);
void Free3d(float*** array, unsigned int x, unsigned int y);
int LoadTexture(Texture* texture, const char* filename);
void CheckOpenglError(void);

/* Functions for 3d board */
extern void InitBoard3d(BoardData* bd, BoardData3d* bd3d);
extern void SetupVisual(void);

typedef struct _RenderToBufferData
{
	unsigned int width;
	unsigned int height;
	BoardData* bd;
	unsigned char* puch;
} RenderToBufferData;
extern gboolean RenderToBuffer3d(GtkWidget* widget, GdkEventExpose* eventData, void* data);
extern void DeleteTextureList(void);

extern void updateOccPos(const BoardData* bd);

extern int ShadowsInitilised(const BoardData3d* bd3d);
void shadowInit(BoardData3d* bd3d, renderdata* prd);
void shadowDisplay(const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd);

/* font functions */
int CreateFonts(BoardData3d* bd3d);
int CreateFontText(OGLFont* ppFont, const char* text, const char* fontFile, int pitch, float size, float heightRatio);
float GetFontHeight3d(const OGLFont* font);
void glPrintPointNumbers(const OGLFont* numberFont, const char* text, int MAA);
void glPrintCube(const OGLFont* cubeFont, const char* text, int MAA);
void glPrintNumbersRA(const OGLFont* numberFont, const char* text);
void glDrawText(const OGLFont* font);
float getTextLen3d(const OGLFont* pFont, const char* str);
extern FT_Pos GetKern(const OGLFont* pFont, char cur, char next);
extern void FreeNumberFont(OGLFont* ppFont);
extern void FreeTextFont(OGLFont* ppFont);

extern int extensionSupported(const char* extension);

void RenderString3d(const OGLFont* pFont, const char* str, float scale, int MAA);
extern void drawTable(const ModelManager* modelHolder, const BoardData3d* bd3d, const renderdata* prd);
extern void getMoveIndicatorPos(int turn, float pos[3]);
extern void drawMoveIndicator(void);
extern void drawFlagPole(unsigned int curveAccuracy);
extern void drawPoint(const renderdata* prd, float tuv, unsigned int i, int p, int outline);
extern void drawBackground(const renderdata* prd, const float* bd3dbackGroundPos, const float* bd3dbackGroundSize);
extern int CreateNumberFont(OGLFont* ppFont, const char* fontFile, int pitch, float size, float heightRatio);

/* Clipping planes */
#define zNearVAL .1f
#define zFarVAL 70.0f

/* Animation paths */
#define MAX_PATHS 3
typedef enum _PathType {
    PATH_LINE, PATH_CURVE_9TO12, PATH_CURVE_12TO3, PATH_PARABOLA, PATH_PARABOLA_12TO3
} PathType;

struct _Path {
    float pts[MAX_PATHS + 1][3];
    PathType pathType[MAX_PATHS];
    int state;
    float mileStone;
    int numSegments;
};

#define FILENAME_SIZE 15
#define NAME_SIZE 20

struct _TextureInfo {
    char file[FILENAME_SIZE + 1];
    char name[NAME_SIZE + 1];
    TextureType type;
};

struct _BoardData3d {
    GtkWidget *drawing_area3d;  /* main 3d widget */

	/* Store models for each board part */
	ModelManager modelHolder;

    /* Bit of a hack - assign each possible position a set rotation */
    int pieceRotation[28][15];
    int movingPieceRotation;

    /* Misc 3d objects */
    Material gapColour;
    Material logoMat;
    Material flagMat, flagNumberMat;

    /* Store how "big" the screen maps to in 3d */
    float backGroundPos[2], backGroundSize[2];

    float perOpen;              /* Percentage open when opening/closing board */

    int moving;                 /* Is a piece moving (animating) */
    Path piecePath;             /* Animation path for moving pieces */
    float rotateMovingPiece;    /* Piece going home? */
    int shakingDice;            /* Are dice being animated */
    Path dicePaths[2];          /* Dice shaking paths */

    /* Some positions of dice, moving/dragging pieces */
    float movingPos[3];
    float dragPos[3];
    float dicePos[2][3];
    float diceMovingPos[2][3];
    DiceRotation diceRotation[2];

    float flagWaved;            /* How much has flag waved */

	OGLFont numberFont, cubeFont;     /* OpenGL fonts */

    /* Saved viewing values - for offscreen render */
    float vertFrustrum, horFrustrum;

    /* Shadow casters */
    Occluder Occluders[/*NUM_OCC*/37];
    float shadow_light_position[4];
    int shadowsInitialised;
    int fBuffers;
    int shadowsOutofDate;

	EigthPoints boardPoints;       /* Used for rounded corners */

    /* Textures */
#define MAX_TEXTURES 10
    Texture textureList[MAX_TEXTURES];
    char *textureName[MAX_TEXTURES];
    int numTextures;
    unsigned int dotTexture;    /* Holds texture used to draw dots on dice */
};

extern struct _Flag3d flag;

/* Define relative sizes of objects from arbitrary unit .05 */
#define base_unit .05f

/* Piece/point size */
#define PIECE_HOLE (base_unit * 3.0f)
#define PIECE_DEPTH base_unit

/* Scale textures by this amount */
#define TEXTURE_SCALE (10.0f / base_unit)

#define copyPoint(to, from) memcpy(to, from, sizeof(float[3]))

#define TEXTURE_PATH "textures/"
#define NO_TEXTURE_STRING _("No texture")

#define HINGE_SEGMENTS 6.f

#define CUBE_TWODIGIT_FACTOR .9f

/* Draw board parts (boxes) specially */
enum { /*boxType */ BOX_ALL = 0, BOX_NOSIDES = 1, BOX_NOENDS = 2, BOX_SPLITTOP = 4, BOX_SPLITWIDTH = 8 };

/* functions used in test harness (static in gnubg) */
#ifndef TEST_HARNESS
#define NTH_STATIC static
#else
#define NTH_STATIC extern
extern void TestHarnessDraw(const BoardData * bd);
#endif

/* Some 3d functions */
extern float getDiceSize(const renderdata * prd);
extern void SetupFlag(void);
extern void setupDicePaths(const BoardData * bd, Path dicePaths[2], float diceMovingPos[2][3],
                           DiceRotation diceRotation[2]);
extern void waveFlag(float wag);

/* Other functions */
void initPath(Path * p, const float start[3]);
void addPathSegment(Path * p, PathType type, const float point[3]);
void initDT(diceTest * dt, int x, int y, int z);

typedef void (*RealizeCB)(void*);
typedef void (*ConfigureCB)(GtkWidget*, void*);
typedef gboolean (*ExposeCB)(GtkWidget*, GdkEventExpose*, void*);
GtkWidget* GLWidgetCreate(RealizeCB realizeCB, ConfigureCB configureCB, ExposeCB exposeCB, void* data);
void GLWidgetMakeCurrent(GtkWidget* widget);
gboolean GLWidgetRender(GtkWidget* widget, ExposeCB exposeCB, GdkEventExpose* eventDetails, void* data);
gboolean GLInit(int* argc, char*** argv);

// Functions required for 3d test harness
void SetupSimpleMatAlpha(Material* pMat, float r, float g, float b, float a);
int movePath(Path* p, float d, float* rotate, float v[3]);

extern int DiceShowing(const BoardData* bd);
extern void getFlagPos(const BoardData* bd, float v[3]);
extern void getDicePos(const BoardData* bd, int num, float v[3]);
extern void getDoubleCubePos(const BoardData* bd, float v[3]);
extern void TidyShadows(BoardData3d* bd3d);
extern void MakeShadowModel(const BoardData* bd, BoardData3d* bd3d, const renderdata* prd);
extern void initOccluders(BoardData3d* bd3d);
extern void UpdateShadowLightPosition(BoardData3d* bd3d, float lp[4]);
extern void getPickMatrices(float x, float y, const BoardData* bd, int* viewport, float** projMat, float** modelMat);
extern void glSetViewport(int viewport[4]);
extern void SetupViewingVolume3d(const BoardData* bd, BoardData3d* bd3d, const renderdata* prd, int viewport[4]);
extern void SetupViewingVolume3dNew(const BoardData* bd, BoardData3d* bd3d, const renderdata* prd, float** projMat, float** modelMat, int viewport[4]);
extern void ClearScreen(const renderdata* prd);
extern void moveToDoubleCubePos(const BoardData* bd);
extern float* GetModelViewMatrix(void);
extern float* GetProjectionMatrix(void);
extern void LegacyStartAA(float width);
extern void LegacyEndAA(void);
extern void ShowMoveIndicator(const ModelManager* modelHolder, const BoardData* bd);
extern void MAAmoveIndicator(void);
extern void drawPiece(const ModelManager* modelHolder, const BoardData3d* bd3d, unsigned int point, unsigned int pos, int rotate, int roundPiece, int curveAccuracy, int separateTop);
extern void MAAdie(const renderdata* prd);
extern void renderFlag(const ModelManager* modelHolder, const BoardData3d* bd3d, int curveAccuracy);
extern void MoveToFlagPos(const BoardData* bd);
extern void MoveToFlagMiddle(void);
extern void PopMatrix(void);
extern void MAApiece(int roundPiece, int curveAccuracy);
extern void renderPiece(const ModelManager* modelHolder, int separateTop);
extern void DrawBackDice(const ModelManager* modelHolder, const BoardData3d* bd3d, const renderdata* prd, diceTest* dt, int diceCol);
extern void DrawDots(const ModelManager* modelHolder, const BoardData3d* bd3d, const renderdata* prd, diceTest* dt, int diceCol);
extern void gluNurbFlagRender(int curveAccuracy);

#endif
