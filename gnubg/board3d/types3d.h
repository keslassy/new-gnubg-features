/*
 * Copyright (C) 2006-2019 Jon Kinsey <jonkinsey@gmail.com>
 * Copyright (C) 2007-2020 the AUTHORS
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

#ifndef TYPES3D_H
#define TYPES3D_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <gtk/gtk.h>

#include "model.h"

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef float GLfloat;

typedef enum _displaytype {
    DT_2D, DT_3D
} displaytype;

typedef enum _lighttype {
    LT_POSITIONAL, LT_DIRECTIONAL
} lighttype;

typedef enum _PieceType {
    PT_ROUNDED, PT_FLAT
} PieceType;

typedef enum _PieceTextureType {
    PTT_TOP, PTT_ALL, PTT_BOTTOM, NUM_TEXTURE_TYPES
} PieceTextureType;

typedef enum _TextureType {
    TT_NONE = 1, TT_GENERAL = 2, TT_PIECE = 4, TT_HINGE = 8, TT_DISABLED = 16
} TextureType;


typedef struct _Texture {
        unsigned int texID;
        int width;
        int height;
} Texture;

#define FILENAME_SIZE 15
#define NAME_SIZE 20

typedef struct _TextureInfo {
    char file[FILENAME_SIZE + 1];
    char name[NAME_SIZE + 1];
    TextureType type;
} TextureInfo;


typedef struct _Material {
    float ambientColour[4];
    float diffuseColour[4];
    float specularColour[4];
    int shine;
    int alphaBlend;
    TextureInfo* textureInfo;
    Texture* pTexture;
} Material;


typedef struct _OglModel
{
        float* data;
        int dataLength;
        int dataStart;
} OglModel;

enum ModelType { MT_BACKGROUND, MT_BASE, MT_ODDPOINTS, MT_EVENPOINTS, MT_TABLE, MT_HINGE, MT_HINGEGAP, MT_CUBE, MT_MOVEINDICATOR, MT_PIECE, MT_PIECETOP, MT_DICE, MT_DOT, MT_FLAG, MT_FLAGPOLE, MT_RECT, MAX_MODELS };

typedef struct _ModelManager
{
    int totalNumVertices;
    int allocNumVertices;
    float* vertexData;
    int numModels;
    OglModel models[MAX_MODELS];
#if GTK_CHECK_VERSION(3,0,0)
    guint vao;
    guint buffer;
#endif
} ModelManager;


/* Animation paths */

#define MAX_PATHS 3
typedef enum _PathType {
    PATH_LINE, PATH_CURVE_9TO12, PATH_CURVE_12TO3, PATH_PARABOLA, PATH_PARABOLA_12TO3
} PathType;

typedef struct _Path {
    float pts[MAX_PATHS + 1][3];
    PathType pathType[MAX_PATHS];
    int state;
    float mileStone;
    int numSegments;
} Path;


/* Dice */

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
{       /* Used for rounded corners */
    float*** points;
    unsigned int accuracy;
} EigthPoints;


typedef struct _OGLFont {
    unsigned int textGlyphs;        // Used in graph
    unsigned int AAglyphs;  // For Anti-aliasing
    FT_Pos advance;
    FT_Pos kern[10][10];
    float scale;
    float heightRatio;
    float height;
    float length;
    ModelManager modelManager;
} OGLFont;


typedef struct _BoardData3d {
    GtkWidget *drawing_area3d;  /* main 3d widget */

    /* Store models for each board part */
    ModelManager modelHolder;

    /* Bit of a hack - assign each possible position a set rotation */
    int pieceRotation[28][15];
    int movingPieceRotation;

    /* Misc 3d objects */
    Material gapColour;
    Material logoMat;
    Material flagMat, flagPoleMat, flagNumberMat;

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
} BoardData3d;

#endif
