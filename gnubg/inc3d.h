/*
 * Copyright (C) 2019 Jon Kinsey <jonkinsey@gmail.com>
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

#ifndef INC3D3D_H
#define INC3D3D_H

#include <glib.h>

typedef struct _BoardData BoardData;
typedef struct _BoardData3d BoardData3d;
typedef struct _Material Material;
typedef struct _TextureInfo TextureInfo;
typedef struct _renderdata renderdata;
typedef struct _Texture Texture;
typedef struct _GraphData GraphData;
typedef struct _statcontext statcontext;
typedef struct _GtkWidget GtkWidget;

#define DF_VARIABLE_OPACITY 1
#define DF_NO_ALPHA 2
#define DF_FULL_ALPHA 4

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
	PTT_TOP, PTT_BOTTOM, PTT_ALL, NUM_TEXTURE_TYPES
} PieceTextureType;

typedef enum _TextureType {
	TT_NONE = 1, TT_GENERAL = 2, TT_PIECE = 4, TT_HINGE = 8, TT_DISABLED = 16
} TextureType;

struct _Material {
	float ambientColour[4];
	float diffuseColour[4];
	float specularColour[4];
	int shine;
	int alphaBlend;
	TextureInfo* textureInfo;
	Texture* pTexture;
};

extern void StopIdle3d(const BoardData* bd, BoardData3d* bd3d);
extern void Tidy3dObjects(BoardData3d* bd3d, const renderdata* prd);
extern int setVSync(int state);
extern char* MaterialGetTextureFilename(const Material* pMat);
extern void FindTexture(TextureInfo** textureInfo, const char* file);
GtkWidget* GetDrawingArea3d(const BoardData3d* bd3d);
extern void updateDiceOccPos(const BoardData* bd, BoardData3d* bd3d);
extern void updatePieceOccPos(const BoardData* bd, BoardData3d* bd3d);
extern void updateHingeOccPos(BoardData3d* bd3d, int show3dHinges);
extern void updateFlagOccPos(const BoardData* bd, BoardData3d* bd3d);
extern void SetMovingPieceRotation(const BoardData* bd, BoardData3d* bd3d, unsigned int pt);
extern int MouseMove3d(const BoardData* bd, BoardData3d* bd3d, const renderdata* prd, int x, int y);
extern void PlaceMovingPieceRotation(const BoardData* bd, BoardData3d* bd3d, unsigned int dest, unsigned int src);
extern void DrawScene3d(const BoardData3d* bd3d);
extern int Animating3d(const BoardData3d* bd3d);
extern void RollDice3d(BoardData* bd, BoardData3d* bd3d, const renderdata* prd);
extern void setDicePos(BoardData* bd, BoardData3d* bd3d);
extern int BoardPoint3d(const BoardData* bd, int x, int y);
extern int BoardSubPoint3d(const BoardData* bd, int x, int y, unsigned int point);
extern void RecalcViewingVolume(const BoardData* bd);
extern void ShowFlag3d(BoardData* bd, BoardData3d* bd3d, const renderdata* prd);
extern void AnimateMove3d(BoardData* bd, BoardData3d* bd3d);
extern void UpdateShadows(BoardData3d* bd3d);
extern int CreateGLWidget(BoardData* bd);
extern void InitColourSelectionDialog(void);
extern void MakeCurrent3d(const BoardData3d* bd3d);
extern void preDraw3d(const BoardData* bd, BoardData3d* bd3d, renderdata* prd);
extern gboolean InitGTK3d(int* argc, char*** argv);
extern void DisplayCorrectBoardType(BoardData* bd, BoardData3d* bd3d, renderdata* prd);
/* Graph functions */
GtkWidget* StatGraph(GraphData* pgd);
void SetNumGames(GraphData* pgd, unsigned int numGames);
void AddGameData(GraphData* pgd, int game, const statcontext* psc);
void TidyGraphData(GraphData* pgd);
GraphData* CreateGraphData(void);

extern void ResetPreviews(void);
extern void UpdateColPreviews(void);
extern int GetPreviewId(void);
extern void UpdateColPreview(int ID);
extern void SetPreviewLightLevel(const int levels[3]);
extern int MaterialCompare(Material* pMat1, Material* pMat2);
extern void GetTextures(BoardData3d* bd3d, renderdata* prd);
extern void ClearTextures(BoardData3d* bd3d);
extern int DiceTooClose(const BoardData3d* bd3d, const renderdata* prd);
extern GtkWidget* gtk_colour_picker_new3d(Material* pMat, int opacity, TextureType textureType);
extern float TestPerformance3d(BoardData* bd);
extern void Set3dSettings(renderdata* prdnew, const renderdata* prd);

#endif
