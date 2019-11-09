/*
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

#ifndef FUN3D_H
#define FUN3D_H

#include "types3d.h"
#include "gtkboard.h"
#include "analysis.h"

/* Setup functions */
void InitGL(const BoardData * bd);

/* Drawing functions */
void drawBoard(const BoardData * bd, const BoardData3d * bd3d, const renderdata * prd);
extern void Draw3d(const BoardData * bd);
void SetupPerspVolume(const BoardData * bd, BoardData3d * bd3d, const renderdata * prd, int viewport[4]);
float getBoardWidth(void);
float getBoardHeight(void);
void calculateBackgroundSize(BoardData3d * bd3d, const int viewport[4]);

extern void getPiecePos(unsigned int point, unsigned int pos, float v[3]);

/* Graph functions */
GtkWidget *StatGraph(GraphData * pgd);
void SetNumGames(GraphData * pgd, unsigned int numGames);
void AddGameData(GraphData * pgd, int game, const statcontext * psc);
void TidyGraphData(GraphData * pgd);
GraphData *CreateGraphData(void);

/* Misc functions */
void SetupSimpleMat(Material * pMat, float r, float g, float b);
void SetupMat(Material * pMat, float r, float g, float b, float dr, float dg, float db, float sr, float sg, float sb,
              int shin, float a);
void setMaterial(const Material * pMat);
void SetColour3d(float r, float g, float b, float a);
float randRange(float range);
void setupPath(const BoardData * bd, Path * p, float *pRotate, unsigned int fromPoint, unsigned int fromDepth,
               unsigned int toPoint, unsigned int toDepth);
int finishedPath(const Path * p);
void getProjectedPieceDragPos(int x, int y, float pos[3]);
void updateMovingPieceOccPos(const BoardData * bd, BoardData3d * bd3d);
void LoadTextureInfo(void);
GList *GetTextureList(TextureType type);
extern void FindTexture(TextureInfo ** textureInfo, const char *file);
extern void FindNamedTexture(TextureInfo ** textureInfo, char *name);
float Dist2d(float a, float b);
float ***Alloc3d(unsigned int x, unsigned int y, unsigned int z);
void Free3d(float ***array, unsigned int x, unsigned int y);
int LoadTexture(Texture *texture, const char *filename);
void CheckOpenglError(void);

/* Functions for 3d board */
extern void InitGTK3d(int *argc, char ***argv);
extern void InitBoard3d(BoardData * bd, BoardData3d * bd3d);
extern void SetupVisual(void);
extern void SetupViewingVolume3d(const BoardData * bd, BoardData3d * bd3d, const renderdata * prd);
extern void DisplayCorrectBoardType(BoardData * bd, BoardData3d * bd3d, renderdata * prd);
extern int CreateGLWidget(BoardData * bd);

extern void RollDice3d(BoardData * bd, BoardData3d * bd3d, const renderdata * prd);
extern void AnimateMove3d(BoardData * bd, BoardData3d * bd3d);
extern void ShowFlag3d(BoardData * bd, BoardData3d * bd3d, const renderdata * prd);
extern void StopIdle3d(const BoardData * bd, BoardData3d * bd3d);
extern void preDraw3d(const BoardData * bd, BoardData3d * bd3d, renderdata * prd);
extern int BoardPoint3d(const BoardData * bd, int x, int y);
extern int BoardSubPoint3d(const BoardData * bd, int x, int y, guint point);
extern int MouseMove3d(const BoardData * bd, BoardData3d * bd3d, const renderdata * prd, int x, int y);
typedef struct _RenderToBufferData
{
	unsigned int width;
	unsigned int height;
	BoardData* bd;
	unsigned char* puch;
} RenderToBufferData;
extern gboolean RenderToBuffer3d(GtkWidget* widget, GdkEventExpose* eventData, const RenderToBufferData* renderToBufferData);
extern void Tidy3dObjects(BoardData3d * bd3d, const renderdata * prd);
extern float TestPerformance3d(BoardData * bd);
extern void Set3dSettings(renderdata * prdnew, const renderdata * prd);
extern void MakeCurrent3d(const BoardData3d * bd3d);
extern void GetTextures(BoardData3d * bd3d, renderdata * prd);
extern void ClearTextures(BoardData3d * bd3d);
extern void DeleteTextureList(void);

extern void PlaceMovingPieceRotation(const BoardData * bd, BoardData3d * bd3d, unsigned int dest, unsigned int src);
extern void SetMovingPieceRotation(const BoardData * bd, BoardData3d * bd3d, unsigned int pt);
extern void UpdateShadows(BoardData3d * bd3d);
extern void updateOccPos(const BoardData * bd);
extern void updateDiceOccPos(const BoardData * bd, BoardData3d * bd3d);
extern void updatePieceOccPos(const BoardData * bd, BoardData3d * bd3d);
extern void updateHingeOccPos(BoardData3d * bd3d, int show3dHinges);
extern void updateFlagOccPos(const BoardData * bd, BoardData3d * bd3d);

extern void setDicePos(BoardData * bd, BoardData3d * bd3d);
extern int DiceTooClose(const BoardData3d * bd3d, const renderdata * prd);

void SuspendDiceRolling(renderdata * prd);
void ResumeDiceRolling(renderdata * prd);

extern int ShadowsInitilised(const BoardData3d * bd3d);
void shadowInit(BoardData3d * bd3d, renderdata * prd);
void shadowDisplay(const BoardData * bd, const BoardData3d * bd3d, const renderdata * prd);

/* font functions */
int CreateFonts(BoardData3d* bd3d);
int CreateFontText(OGLFont* ppFont, const char* text, const char* fontFile, int pitch, float size, float heightRatio); 
float GetFontHeight3d(const OGLFont* font);
void glPrintPointNumbers(const OGLFont* numberFont, const char* text, int MAA);
void glPrintCube(const OGLFont * cubeFont, const char *text, int MAA);
void glPrintNumbersRA(const OGLFont* numberFont, const char* text);
void glDrawText(const OGLFont* font);
float getTextLen3d(const OGLFont* pFont, const char* str);
extern FT_Pos GetKern(const OGLFont* pFont, char cur, char next);
extern void FreeNumberFont(OGLFont* ppFont);
extern void FreeTextFont(OGLFont* ppFont);

GtkWidget *GetDrawingArea3d(const BoardData3d * bd3d);
extern int MaterialCompare(Material * pMat1, Material * pMat2);
extern char *MaterialGetTextureFilename(const Material * pMat);
extern void DrawScene3d(const BoardData3d * bd3d);
extern int Animating3d(const BoardData3d * bd3d);

extern void ResetPreviews(void);
extern void UpdateColPreviews(void);
extern int GetPreviewId(void);
extern void UpdateColPreview(int ID);
extern void SetPreviewLightLevel(const int levels[3]);
extern gboolean display_is_2d(const renderdata * prd);
extern gboolean display_is_3d(const renderdata * prd);
extern int setVSync(int state);
extern int extensionSupported(const char *extension);

extern void InitColourSelectionDialog(void);
extern GtkWidget *gtk_colour_picker_new3d(Material * pMat, int opacity, TextureType textureType);

void RenderString3d(const OGLFont* pFont, const char* str, float scale, int MAA);
extern void drawTable(const OglModelHolder* modelHolder, const BoardData3d* bd3d, const renderdata* prd);
extern void getMoveIndicatorPos(int turn, float pos[3]);
extern void drawMoveIndicator();
extern void drawFlagPole(unsigned int curveAccuracy);
extern void drawPoint(const renderdata* prd, float tuv, unsigned int i, int p, int outline);
extern void drawBackground(const renderdata* prd, const float* bd3dbackGroundPos, const float* bd3dbackGroundSize);
extern int CreateNumberFont(OGLFont* ppFont, const char* fontFile, int pitch, float size, float heightRatio);

#endif
