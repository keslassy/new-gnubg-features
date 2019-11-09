/*
 * inc3d.h
 * by Jon Kinsey, 2003
 *
 * General definitions for 3d board
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 or later of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */
#ifndef INC3D_H
#define INC3D_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include "fun3d.h"
#include "matrix.h"
#include "gtkwindows.h"

#include "model.h"
#include "drawboard.h"          /* for fClockwise decl */

/* Clipping planes */
#define zNear .1
#define zFar 70.0

extern gboolean gtk_gl_init_success;

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
	OglModelHolder modelHolder;

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

    /* Saved viewing values (used for picking) */
    double vertFrustrum, horFrustrum;
    float modelMatrix[16];

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

#endif
