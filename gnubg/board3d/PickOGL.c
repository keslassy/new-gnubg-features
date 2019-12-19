
#include "config.h"
#include "legacyGLinc.h"
#include "fun3d.h"
#include "BoardDimensions.h"
#include "boardpos.h"
#include "gtkboard.h"

extern Flag3d flag;
extern void getFlagPos(const BoardData* bd, float v[3]);
extern void moveToDicePos(const BoardData* bd, int num);

#include "Shapes.inc"

extern void
drawFlagPick(const BoardData* bd, void* UNUSED(data))
{
	BoardData3d* bd3d = bd->bd3d;
	renderdata* prd = bd->rd;
	int s;
	float v[3];

	waveFlag(bd3d->flagWaved);

	glLoadName(POINT_RESIGN);

	glPushMatrix();

	getFlagPos(bd, v);
	glTranslatef(v[0], v[1], v[2]);

	/* Draw flag surface (approximation) */
	glBegin(GL_QUAD_STRIP);
	for (s = 0; s < /*S_NUMPOINTS*/4; s++) {
		glVertex3fv(flag.ctlpoints[s][1]);
		glVertex3fv(flag.ctlpoints[s][0]);
	}
	glEnd();

	/* Draw flag pole */
	glTranslatef(0.f, -FLAG_HEIGHT, 0.f);

	drawRect(-FLAGPOLE_WIDTH * 2, 0, 0, FLAGPOLE_WIDTH * 2, FLAGPOLE_HEIGHT, NULL);
	glRotatef(-90.f, 1.f, 0.f, 0.f);

	circleRev(FLAGPOLE_WIDTH, 0.f, prd->curveAccuracy);
	circleRev(FLAGPOLE_WIDTH * 2, FLAGPOLE_HEIGHT, prd->curveAccuracy);

	glPopMatrix();
}

NTH_STATIC void
drawPickAreas(const BoardData* bd, void* UNUSED(data))
{                               /* Draw main board areas to see where user clicked */
	renderdata* prd = bd->rd;
	float barHeight;

	/* boards */
	glLoadName(fClockwise ? 2 : 1);
	drawRect(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH, EDGE_HEIGHT, BASE_DEPTH, PIECE_HOLE * 6, POINT_HEIGHT, 0);

	glLoadName(fClockwise ? 1 : 2);
	drawRect(TRAY_WIDTH, EDGE_HEIGHT, BASE_DEPTH, PIECE_HOLE * 6, POINT_HEIGHT, 0);

	glLoadName(fClockwise ? 4 : 3);
	drawRect(TRAY_WIDTH + BOARD_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH, -PIECE_HOLE * 6, -POINT_HEIGHT, 0);

	glLoadName(fClockwise ? 3 : 4);
	drawRect(TOTAL_WIDTH - TRAY_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH, -PIECE_HOLE * 6, -POINT_HEIGHT, 0);

	/* bars + homes */
	barHeight = (PIECE_HOLE + PIECE_GAP_HEIGHT) * 4;
	glLoadName(0);
	drawRect(TOTAL_WIDTH / 2.0f - PIECE_HOLE / 2.0f,
		TOTAL_HEIGHT / 2.0f - (DOUBLECUBE_SIZE / 2.0f + barHeight + PIECE_HOLE / 2.0f), BASE_DEPTH + EDGE_DEPTH,
		PIECE_HOLE, barHeight, 0);
	glLoadName(25);
	drawRect(TOTAL_WIDTH / 2.0f - PIECE_HOLE / 2.0f, TOTAL_HEIGHT / 2.0f + (DOUBLECUBE_SIZE / 2.0f + PIECE_HOLE / 2.0f),
		BASE_DEPTH + EDGE_DEPTH, PIECE_HOLE, barHeight, 0);

	/* Home/unused trays depend on direction */
	glLoadName((!fClockwise) ? 26 : POINT_UNUSED0);
	drawRect(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH, TRAY_WIDTH - EDGE_WIDTH * 2,
		TRAY_HEIGHT - EDGE_HEIGHT, 0);
	glLoadName((fClockwise) ? 26 : POINT_UNUSED0);
	drawRect(EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH, TRAY_WIDTH - EDGE_WIDTH * 2, TRAY_HEIGHT - EDGE_HEIGHT, 0);

	glLoadName((!fClockwise) ? 27 : POINT_UNUSED1);
	drawRect(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT - POINT_HEIGHT, BASE_DEPTH,
		TRAY_WIDTH - EDGE_WIDTH * 2, POINT_HEIGHT, 0);
	glLoadName((fClockwise) ? 27 : POINT_UNUSED1);
	drawRect(EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT - POINT_HEIGHT, BASE_DEPTH, TRAY_WIDTH - EDGE_WIDTH * 2,
		POINT_HEIGHT, 0);

	/* Dice areas */
	glLoadName(POINT_LEFT);
	drawRect(TRAY_WIDTH, (TOTAL_HEIGHT - DICE_AREA_HEIGHT) / 2.0f, BASE_DEPTH, DICE_AREA_CLICK_WIDTH, DICE_AREA_HEIGHT,
		0);
	glLoadName(POINT_RIGHT);
	drawRect(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH, (TOTAL_HEIGHT - DICE_AREA_HEIGHT) / 2.0f, BASE_DEPTH,
		DICE_AREA_CLICK_WIDTH, DICE_AREA_HEIGHT, 0);

	/* Dice */
	if (DiceShowing(bd)) {
		glLoadName(POINT_DICE);
		glPushMatrix();
		moveToDicePos(bd, 0);
		drawCube(getDiceSize(prd) * .95f);
		glPopMatrix();
		glPushMatrix();
		moveToDicePos(bd, 1);
		drawCube(getDiceSize(prd) * .95f);
		glPopMatrix();
	}

	/* Double Cube */
	glPushMatrix();
	glLoadName(POINT_CUBE);
	moveToDoubleCubePos(bd);
	drawCube(DOUBLECUBE_SIZE * .95f);
	glPopMatrix();
}

static void
drawPieceSimplified(const BoardData3d* UNUSED(bd3d), unsigned int point, unsigned int pos)
{
	float v[3];
	glPushMatrix();

	getPiecePos(point, pos, v);
	drawBox(BOX_ALL, v[0] - PIECE_HOLE / 2.f, v[1] - PIECE_HOLE / 2.f, v[2] - PIECE_DEPTH / 2.f, PIECE_HOLE, PIECE_HOLE,
		PIECE_DEPTH, NULL);

	glPopMatrix();
}

NTH_STATIC void
drawPickBoard(const BoardData* bd, void* data)
{                               /* Draw points and piece objects for selected board */
	BoardData3d* bd3d = bd->bd3d;
	int selectedBoard = GPOINTER_TO_INT(data);
	unsigned int i, j;

	unsigned int firstPoint = (selectedBoard - 1) * 6;
	/* pieces */
	for (i = firstPoint + 1; i <= firstPoint + 6; i++) {
		glLoadName(i);
		for (j = 1; j <= Abs(bd->points[i]); j++)
			drawPieceSimplified(bd3d, i, j);
	}
	/* points */
	if (fClockwise) {
		int SwapBoard[4] = { 2, 1, 4, 3 };
		selectedBoard = SwapBoard[selectedBoard - 1];
	}
	for (i = 0; i < 6; i++) {
		switch (selectedBoard) {
		case 1:
			glLoadName(fClockwise ? i + 7 : 6 - i);
			drawRect(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH + PIECE_HOLE * i, EDGE_HEIGHT, BASE_DEPTH, PIECE_HOLE,
				POINT_HEIGHT, 0);
			break;
		case 2:
			glLoadName(fClockwise ? i + 1 : 12 - i);
			drawRect(TRAY_WIDTH + PIECE_HOLE * i, EDGE_HEIGHT, BASE_DEPTH, PIECE_HOLE, POINT_HEIGHT, 0);
			break;
		case 3:
			glLoadName(fClockwise ? i + 19 : 18 - i);
			drawRect(TRAY_WIDTH + BOARD_WIDTH - (PIECE_HOLE * i), TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH, -PIECE_HOLE,
				-POINT_HEIGHT, 0);
			break;
		case 4:
			glLoadName(fClockwise ? i + 13 : 24 - i);
			drawRect(TOTAL_WIDTH - TRAY_WIDTH - (PIECE_HOLE * i), TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH, -PIECE_HOLE,
				-POINT_HEIGHT, 0);
			break;
		}
	}
}

static void
DrawPickIndividualPoint(const BoardData* bd, int point)
{
	BoardData3d* bd3d = bd->bd3d;
	unsigned int j;
	glLoadName(point);
	for (j = 1; j <= Abs(bd->points[point]); j++)
		drawPiece(&bd3d->modelHolder, bd3d, point, j, FALSE, (bd->rd->pieceType == PT_ROUNDED), bd->rd->curveAccuracy, FALSE);

	if (fClockwise) {
		point--;
		if (point < 6)
			drawRect(TRAY_WIDTH + PIECE_HOLE * point, EDGE_HEIGHT, BASE_DEPTH, PIECE_HOLE, POINT_HEIGHT, 0);
		else if (point < 12)
			drawRect(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH + PIECE_HOLE * (point - 6), EDGE_HEIGHT, BASE_DEPTH,
				PIECE_HOLE, POINT_HEIGHT, 0);
		else if (point < 18)
			drawRect(TOTAL_WIDTH - TRAY_WIDTH - (PIECE_HOLE * (point - 12)), TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH,
				-PIECE_HOLE, -POINT_HEIGHT, 0);
		else if (point < 24)
			drawRect(TRAY_WIDTH + BOARD_WIDTH - (PIECE_HOLE * (point - 18)), TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH,
				-PIECE_HOLE, -POINT_HEIGHT, 0);
	}
	else {
		if (point <= 6)
			drawRect(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH + PIECE_HOLE * (6 - point), EDGE_HEIGHT, BASE_DEPTH,
				PIECE_HOLE, POINT_HEIGHT, 0);
		else if (point <= 12)
			drawRect(TRAY_WIDTH + PIECE_HOLE * (12 - point), EDGE_HEIGHT, BASE_DEPTH, PIECE_HOLE, POINT_HEIGHT, 0);
		else if (point <= 18)
			drawRect(TRAY_WIDTH + BOARD_WIDTH - (PIECE_HOLE * (18 - point)), TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH,
				-PIECE_HOLE, -POINT_HEIGHT, 0);
		else if (point <= 24)
			drawRect(TOTAL_WIDTH - TRAY_WIDTH - (PIECE_HOLE * (24 - point)), TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH,
				-PIECE_HOLE, -POINT_HEIGHT, 0);
	}
}

NTH_STATIC void
drawPickPoint(const BoardData* bd, void* data)
{                               /* Draw two points to see which is selected */
	int pointA = (GPOINTER_TO_INT(data)) / 100;
	int pointB = (GPOINTER_TO_INT(data)) - pointA * 100;

	/* pieces on both points */
	DrawPickIndividualPoint(bd, pointA);
	DrawPickIndividualPoint(bd, pointB);
}

/* 20 allows for 5 hit records (more than enough) */
#define BUFSIZE 20
static unsigned int selectBuf[BUFSIZE];

typedef void (*PickDrawFun) (const BoardData* bd, void* data);

static int
PickDraw(int x, int y, PickDrawFun drawFun, const BoardData* bd, void* data)
{                               /* Identify if anything is below point (x,y) */
	int hits;
	int viewport[4];
	float *projMat, *modelMat;

	glSelectBuffer(BUFSIZE, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	glGetIntegerv(GL_VIEWPORT, viewport);

	getPickMatrices((float)x, (float)y, bd, viewport, &projMat, &modelMat);

	/* Setup openGL legacy matrices */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(projMat);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelMat);

	drawFun(bd, data);

	glPopName();

	hits = glRenderMode(GL_RENDER);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	PopMatrix();
	return hits;
}

extern void
drawPointPick(const BoardData* UNUSED(bd), void* data)
{                               /* Draw sub parts of point to work out which part of point clicked */
	unsigned int point = GPOINTER_TO_UINT(data);

	if (point <= 25) {          /* Split first point into 2 parts for zero and one selection */
#define SPLIT_PERCENT .40f
#define SPLIT_HEIGHT (PIECE_HOLE * SPLIT_PERCENT)
		unsigned int i;
		float pos[3];

		getPiecePos(point, 1, pos);

		glLoadName(0);
		if (point > 12)
			drawRect(pos[0] - (PIECE_HOLE / 2.0f), pos[1] + (PIECE_HOLE / 2.0f) + PIECE_GAP_HEIGHT - SPLIT_HEIGHT,
				pos[2], PIECE_HOLE, SPLIT_HEIGHT, 0);
		else
			drawRect(pos[0] - (PIECE_HOLE / 2.0f), pos[1] - (PIECE_HOLE / 2.0f), pos[2], PIECE_HOLE, SPLIT_HEIGHT, 0);

		glLoadName(1);
		if (point > 12)
			drawRect(pos[0] - (PIECE_HOLE / 2.0f), pos[1] - (PIECE_HOLE / 2.0f), pos[2], PIECE_HOLE,
				PIECE_HOLE + PIECE_GAP_HEIGHT - SPLIT_HEIGHT, 0);
		else
			drawRect(pos[0] - (PIECE_HOLE / 2.0f), pos[1] - (PIECE_HOLE / 2.0f) + SPLIT_HEIGHT, pos[2], PIECE_HOLE,
				PIECE_HOLE + PIECE_GAP_HEIGHT - SPLIT_HEIGHT, 0);

		for (i = 2; i < 6; i++) {
			/* Only 3 places for bar */
			if ((point == 0 || point == 25) && i == 4)
				break;
			getPiecePos(point, i, pos);
			glLoadName(i);
			drawRect(pos[0] - (PIECE_HOLE / 2.0f), pos[1] - (PIECE_HOLE / 2.0f), pos[2], PIECE_HOLE,
				PIECE_HOLE + PIECE_GAP_HEIGHT, 0);
		}
		/* extra bit */
		/*              glLoadName(i);
		 * if (point > 12)
		 * {
		 * pos[1] = pos[1] - (PIECE_HOLE / 2.0f);
		 * drawRect(pos[0] - (PIECE_HOLE / 2.0f), TOTAL_HEIGHT - EDGE_HEIGHT - POINT_HEIGHT, pos[2], PIECE_HOLE, pos[1] - (TOTAL_HEIGHT - EDGE_HEIGHT - POINT_HEIGHT), 0);
		 * }
		 * else
		 * {
		 * pos[1] = pos[1] + (PIECE_HOLE / 2.0f) + PIECE_GAP_HEIGHT;
		 * drawRect(pos[0] - (PIECE_HOLE / 2.0f), pos[1], pos[2], PIECE_HOLE, EDGE_HEIGHT + POINT_HEIGHT - pos[1], 0);
		 * }
		 */
	}
}

static int
NearestHit(int hits, const unsigned int* ptr)
{
	if (hits == 1) {            /* Only one hit - just return this one */
		return (int)ptr[3];
	}
	else {                    /* Find the highest/closest object */
		int i, sel = -1;
		unsigned int names;
		float minDepth = FLT_MAX;

		for (i = 0; i < hits; i++) {    /* for each hit */
			float depth;

			names = *ptr++;
			depth = (float)*ptr++ / 0x7fffffff;
			ptr++;              /* Skip max depth value */
			/* Ignore clicks on the board base as other objects must be closer */
			if (*ptr >= POINT_DICE && !(*ptr == POINT_LEFT || *ptr == POINT_RIGHT) && depth < minDepth) {
				minDepth = depth;
				sel = (int)*ptr;
			}
			ptr += names;
		}
		return sel;
	}
}

int
BoardSubPoint3d(const BoardData* bd, int x, int y, unsigned int point)
{
	int hits = PickDraw(x, y, drawPointPick, bd, GUINT_TO_POINTER(point));
	return NearestHit(hits, (unsigned int*)selectBuf);
}

int
BoardPoint3d(const BoardData* bd, int x, int y)
{
	int hits;
	if (bd->resigned) {         /* Flag showing - just pick this */
		hits = PickDraw(x, y, drawFlagPick, bd, NULL);
		return NearestHit(hits, (unsigned int*)selectBuf);
	}
	else {
		int picked, firstPoint, secondPoint;
		hits = PickDraw(x, y, drawPickAreas, bd, NULL);
		picked = NearestHit(hits, (unsigned int*)selectBuf);
		if (picked <= 0 || picked > 24)
			return picked;      /* Not a point so actual hit */
		g_assert(picked >= 1 && picked <= 4);   /* one of 4 boards */

		/* Work out which point in board was clicked */
		hits = PickDraw(x, y, drawPickBoard, bd, GINT_TO_POINTER(picked));
		firstPoint = (int)selectBuf[3];
		if (hits == 1)
			return firstPoint;  /* Board point clicked */

		secondPoint = (int)selectBuf[selectBuf[0] + 6];
		if (firstPoint == secondPoint)
			return firstPoint;  /* Chequer clicked over point (common) */

		/* Could be either chequer or point - work out which one */
		hits = PickDraw(x, y, drawPickPoint, bd, GINT_TO_POINTER(firstPoint * 100 + secondPoint));
		return NearestHit(hits, (unsigned int*)selectBuf);
	}
}
