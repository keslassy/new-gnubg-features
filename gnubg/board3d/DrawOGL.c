
#include "config.h"
#include "legacyGLinc.h"
#include "inc3d.h"
#include "BoardDimensions.h"

extern void drawTableGrayed(const OglModelHolder* modelHolder, const BoardData3d* bd3d, renderdata tmp);
extern int LogCube(unsigned int n);
extern void WorkOutViewArea(const BoardData* bd, viewArea* pva, float* pHalfRadianFOV, float aspectRatio);
extern float getAreaRatio(const viewArea* pva);
extern float getViewAreaHeight(const viewArea* pva);
static void drawNumbers(const BoardData* bd, int MAA);
static void MAAtidyEdges(const renderdata* prd);
extern void drawDC(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd);
static void MAAmoveIndicator();
extern void drawPieces(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd);
extern void drawDie(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, int num);
static void drawSpecialPieces(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd);
static void drawFlag(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd);
extern void drawDice(const OglModelHolder* modelHolder, const BoardData* bd, int num, renderdata* prd);

///////////////////////////////////////
// Legacy Opengl board rendering code

#include "Shapes.inc"

void
drawBoard(const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd)
{
	const OglModelHolder* modelHolder = &bd3d->modelHolder;

	if (!bd->grayBoard)
		drawTable(modelHolder, bd3d, prd);
	else
		drawTableGrayed(modelHolder, bd3d, *prd);

	if (prd->fLabels)
	{
		drawNumbers(bd, 0);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		drawNumbers(bd, 1);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
	}

	//    if (bd3d->State == BOARD_OPEN)
	MAAtidyEdges(prd);

	if (bd->cube_use && !bd->crawford_game)
		drawDC(modelHolder, bd, bd3d, prd);

	if (prd->showMoveIndicator)
	{
		float pos[3];

		setMaterial(&bd->rd->ChequerMat[(bd->turn == 1) ? 1 : 0]);

		glDisable(GL_DEPTH_TEST);

		glPushMatrix();
		getMoveIndicatorPos(bd->turn, pos);
		glTranslatef(pos[0], pos[1], pos[2]);
		if (fClockwise)
			glRotatef(180.f, 0.f, 0.f, 1.f);

		OglModelDraw(&modelHolder->moveIndicator);

		//TODO: Enlarge and draw black outline before overlaying arrow in modern OGL...
		MAAmoveIndicator();

		glPopMatrix();

		glEnable(GL_DEPTH_TEST);
	}

	/* Draw things in correct order so transparency works correctly */
	/* First pieces, then dice, then moving pieces */

	drawPieces(modelHolder, bd, bd3d, prd);

	if (DiceShowing(bd)) {
		drawDie(modelHolder, bd, bd3d, 0);
		drawDie(modelHolder, bd, bd3d, 1);
	}

	if (bd3d->moving || bd->drag_point >= 0)
		drawSpecialPieces(modelHolder, bd, bd3d, prd);

	if (bd->resigned)
		drawFlag(modelHolder, bd, bd3d, prd);
}

extern void
drawDCNumbers(const BoardData* bd, const diceTest* dt, int MAA)
{
	int c;
	float radius = DOUBLECUBE_SIZE / 7.0f;
	float ds = (DOUBLECUBE_SIZE * 5.0f / 7.0f);
	float hds = (ds / 2);
	float depth = hds + radius;

	const char* sides[] = { "4", "16", "32", "64", "8", "2" };
	int side;

	glLineWidth(.5f);
	glPushMatrix();
	for (c = 0; c < 6; c++) {
		int nice;

		if (c < 3)
			side = c;
		else
			side = 8 - c;
		/* Nicer top numbers */
		nice = (side == dt->top);

		/* Don't draw bottom number or back number (unless cube at bottom) */
		if (side != dt->bottom && !(side == dt->side[0] && bd->cube_owner != -1)) {
			if (nice)
				glDisable(GL_DEPTH_TEST);

			glPushMatrix();
			glTranslatef(0.f, 0.f, depth + (nice ? 0 : LIFT_OFF));

			glPrintCube(&bd->bd3d->cubeFont, sides[side], MAA);

			glPopMatrix();
			if (nice)
				glEnable(GL_DEPTH_TEST);
		}
		if (c % 2 == 0)
			glRotatef(-90.f, 0.f, 1.f, 0.f);
		else
			glRotatef(90.f, 1.f, 0.f, 0.f);
	}
	glPopMatrix();
}

static void
DrawDCNumbers(const BoardData* bd)
{
	diceTest dt;
	int extraRot = 0;
	int rotDC[6][3] = { {1, 0, 0}, {2, 0, 3}, {0, 0, 0}, {0, 3, 1}, {0, 1, 0}, {3, 0, 3} };

	int cubeIndex;
	/* Rotate to correct number + rotation */
	if (!bd->doubled) {
		cubeIndex = LogCube(bd->cube);
		extraRot = bd->cube_owner + 1;
	}
	else {
		cubeIndex = LogCube(bd->cube * 2);      /* Show offered cube value */
		extraRot = bd->turn + 1;
	}
	if (cubeIndex == 6)
		cubeIndex = 0;	// 0 == show 64

	glRotatef((rotDC[cubeIndex][2] + extraRot) * 90.0f, 0.f, 0.f, 1.f);
	glRotatef(rotDC[cubeIndex][0] * 90.0f, 1.f, 0.f, 0.f);
	glRotatef(rotDC[cubeIndex][1] * 90.0f, 0.f, 1.f, 0.f);

	initDT(&dt, rotDC[cubeIndex][0], rotDC[cubeIndex][1], rotDC[cubeIndex][2] + extraRot);

	setMaterial(&bd->rd->CubeNumberMat);
	glNormal3f(0.f, 0.f, 1.f);

	drawDCNumbers(bd, &dt, 0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	drawDCNumbers(bd, &dt, 1);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

extern void
moveToDoubleCubePos(const BoardData* bd)
{
	float v[3];
	getDoubleCubePos(bd, v);
	glTranslatef(v[0], v[1], v[2]);
}

extern void
moveToDicePos(const BoardData* bd, int num)
{
	float v[3];
	getDicePos(bd, num, v);
	glTranslatef(v[0], v[1], v[2]);

	if (bd->diceShown == DICE_ON_BOARD) {       /* Spin dice to required rotation if on board */
		glRotatef(bd->bd3d->dicePos[num][2], 0.f, 0.f, 1.f);
	}
}

/* Define position of dots on dice */
static int dots1[] = { 2, 2, 0 };
static int dots2[] = { 1, 1, 3, 3, 0 };
static int dots3[] = { 1, 3, 2, 2, 3, 1, 0 };
static int dots4[] = { 1, 1, 1, 3, 3, 1, 3, 3, 0 };
static int dots5[] = { 1, 1, 1, 3, 2, 2, 3, 1, 3, 3, 0 };
static int dots6[] = { 1, 1, 1, 3, 2, 1, 2, 3, 3, 1, 3, 3, 0 };
static int* dots[6] = { dots1, dots2, dots3, dots4, dots5, dots6 };
static int dot_pos[] = { 0, 20, 50, 80 };       /* percentages across face */

static void
drawDots(const BoardData3d* bd3d, float diceSize, float dotOffset, const diceTest* dt, int showFront)
{
	int dot;
	int c;
	int* dp;
	float radius;
	float ds = (diceSize * 5.0f / 7.0f);
	float hds = (ds / 2);
	float x, y;
	float dotSize = diceSize / 10.0f;
	/* Remove specular effects */
	float zero[4] = { 0, 0, 0, 0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, zero);

	radius = diceSize / 7.0f;

	glPushMatrix();
	for (c = 0; c < 6; c++) {
		int nd;

		if (c < 3)
			dot = c;
		else
			dot = 8 - c;

		/* Make sure top dot looks nice */
		nd = !bd3d->shakingDice && (dot == dt->top);

		if (bd3d->shakingDice || (showFront && dot != dt->bottom && dot != dt->side[0])
			|| (!showFront && dot != dt->top && dot != dt->side[2])) {
			if (nd)
				glDisable(GL_DEPTH_TEST);
			glPushMatrix();
			glTranslatef(0.f, 0.f, hds + radius);

			glNormal3f(0.f, 0.f, 1.f);

			/* Show all the dots for this number */
			dp = dots[dot];
			do {
				x = (dot_pos[dp[0]] * ds) / 100;
				y = (dot_pos[dp[1]] * ds) / 100;

				glPushMatrix();
				glTranslatef(x - hds, y - hds, 0.f);

				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, bd3d->dotTexture);

				glBegin(GL_QUADS);
				glTexCoord2f(0.f, 1.f);
				glVertex3f(dotSize, dotSize, dotOffset);
				glTexCoord2f(1.f, 1.f);
				glVertex3f(-dotSize, dotSize, dotOffset);
				glTexCoord2f(1.f, 0.f);
				glVertex3f(-dotSize, -dotSize, dotOffset);
				glTexCoord2f(0.f, 0.f);
				glVertex3f(dotSize, -dotSize, dotOffset);
				glEnd();

				glPopMatrix();

				dp += 2;
			} while (*dp);

			glPopMatrix();
			if (nd)
				glEnable(GL_DEPTH_TEST);
		}

		if (c % 2 == 0)
			glRotatef(-90.f, 0.f, 1.f, 0.f);
		else
			glRotatef(90.f, 1.f, 0.f, 0.f);
	}
	glPopMatrix();
}

extern void
glPrintPointNumbers(const OGLFont* numberFont, const char* text, int MAA)
{
	/* Align horizontally */
	glPushMatrix();
	glTranslatef(-getTextLen3d(numberFont, text) / 2.0f, 0.f, 0.f);
	RenderString3d(numberFont, text, numberFont->scale, MAA);
	glPopMatrix();
}

static void
DrawNumbers(const BoardData* bd, unsigned int sides, int MAA)
{
	int i;
	char num[3];
	float x;
	float textHeight = GetFontHeight3d(&bd->bd3d->numberFont);
	int n;

	glPushMatrix();
	glTranslatef(0.f, (EDGE_HEIGHT - textHeight) / 2.0f, BASE_DEPTH + EDGE_DEPTH);
	x = TRAY_WIDTH - PIECE_HOLE / 2.0f;

	for (i = 0; i < 12; i++) {
		x += PIECE_HOLE;
		if (i == 6)
			x += BAR_WIDTH;

		if ((i < 6 && (sides & 1)) || (i >= 6 && (sides & 2))) {
			glPushMatrix();
			glTranslatef(x, 0.f, 0.f);
			if (!fClockwise)
				n = 12 - i;
			else
				n = i + 1;

			if (bd->turn == -1 && bd->rd->fDynamicLabels)
				n = 25 - n;

			sprintf(num, "%d", n);
			glPrintPointNumbers(&bd->bd3d->numberFont, num, MAA);

			glPopMatrix();
		}
	}

	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.f, TOTAL_HEIGHT - textHeight - (EDGE_HEIGHT - textHeight) / 2.0f, BASE_DEPTH + EDGE_DEPTH);
	x = TRAY_WIDTH - PIECE_HOLE / 2.0f;

	for (i = 0; i < 12; i++) {
		x += PIECE_HOLE;
		if (i == 6)
			x += BAR_WIDTH;
		if ((i < 6 && (sides & 1)) || (i >= 6 && (sides & 2))) {
			glPushMatrix();
			glTranslatef(x, 0.f, 0.f);
			if (!fClockwise)
				n = 13 + i;
			else
				n = 24 - i;

			if (bd->turn == -1 && bd->rd->fDynamicLabels)
				n = 25 - n;

			sprintf(num, "%d", n);
			glPrintPointNumbers(&bd->bd3d->numberFont, num, MAA);

			glPopMatrix();
		}
	}
	glPopMatrix();
}

static void
drawNumbers(const BoardData* bd, int MAA)
{
	/* No need to depth test as on top of board (depth test could cause alias problems too) */
	glDisable(GL_DEPTH_TEST);
	/* Draw inside then anti-aliased outline of numbers */
	setMaterial(&bd->rd->PointNumberMat);
	glNormal3f(0.f, 0.f, 1.f);

	glLineWidth(1.f);
	DrawNumbers(bd, 1, MAA);
	DrawNumbers(bd, 2, MAA);
	glEnable(GL_DEPTH_TEST);
}

static void
MAAtidyEdges(const renderdata* prd)
{                               /* Anti-alias board edges */
	setMaterial(&prd->BoxMat);

	glLineWidth(1.f);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glDepthMask(GL_FALSE);

	glNormal3f(0.f, 0.f, 1.f);

	glBegin(GL_LINES);
	if (prd->roundedEdges) {
		/* bar */
		glNormal3f(-1.f, 0.f, 0.f);
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);

		glNormal3f(1.f, 0.f, 0.f);
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT,
			BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);

		/* left bear off tray */
		glNormal3f(-1.f, 0.f, 0.f);
		glVertex3f(0.f, BOARD_FILLET, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(0.f, TOTAL_HEIGHT - BOARD_FILLET, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);

		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, TRAY_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, TRAY_HEIGHT + MID_SIDE_GAP_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);

		/* right bear off tray */
		glNormal3f(1.f, 0.f, 0.f);
		glVertex3f(TOTAL_WIDTH, BOARD_FILLET, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TOTAL_WIDTH, TOTAL_HEIGHT - BOARD_FILLET, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);

		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TRAY_HEIGHT, BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TRAY_HEIGHT + MID_SIDE_GAP_HEIGHT,
			BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT,
			BASE_DEPTH + EDGE_DEPTH - BOARD_FILLET);

	}
	else {
		/* bar */
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TRAY_WIDTH + BOARD_WIDTH + BAR_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		/* left bear off tray */
		glVertex3f(0.f, 0.f, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(0.f, TOTAL_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(EDGE_WIDTH, TRAY_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(EDGE_WIDTH, TRAY_HEIGHT + MID_SIDE_GAP_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, TRAY_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, TRAY_HEIGHT + MID_SIDE_GAP_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TRAY_WIDTH - EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(TRAY_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TRAY_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		/* right bear off tray */
		glVertex3f(TOTAL_WIDTH, 0.f, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH, TOTAL_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(TOTAL_WIDTH - EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - EDGE_WIDTH, TRAY_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - EDGE_WIDTH, TRAY_HEIGHT + MID_SIDE_GAP_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TRAY_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TRAY_HEIGHT + MID_SIDE_GAP_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + EDGE_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);

		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH, EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
		glVertex3f(TOTAL_WIDTH - TRAY_WIDTH, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + EDGE_DEPTH);
	}

	/* inner edges (sides) */
	glNormal3f(1.f, 0.f, 0.f);
	glVertex3f(EDGE_WIDTH + LIFT_OFF, EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glVertex3f(EDGE_WIDTH + LIFT_OFF, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glVertex3f(TRAY_WIDTH + LIFT_OFF, EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glVertex3f(TRAY_WIDTH + LIFT_OFF, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);

	glNormal3f(-1.f, 0.f, 0.f);
	glVertex3f(TOTAL_WIDTH - EDGE_WIDTH + LIFT_OFF, EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glVertex3f(TOTAL_WIDTH - EDGE_WIDTH + LIFT_OFF, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + LIFT_OFF, EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glVertex3f(TOTAL_WIDTH - TRAY_WIDTH + LIFT_OFF, TOTAL_HEIGHT - EDGE_HEIGHT, BASE_DEPTH + LIFT_OFF);
	glEnd();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

static void
MAAmoveIndicator()
{
	/* Outline arrow */
	SetColour3d(0.f, 0.f, 0.f, 1.f);    /* Black outline */

	glLineWidth(.5f);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);

	glBegin(GL_LINE_LOOP);
	glVertex2f(-ARROW_UNIT * 2, -ARROW_UNIT);
	glVertex2f(-ARROW_UNIT * 2, ARROW_UNIT);
	glVertex2f(0.f, ARROW_UNIT);
	glVertex2f(0.f, ARROW_UNIT * 2);
	glVertex2f(ARROW_UNIT * 2, 0.f);
	glVertex2f(0.f, -ARROW_UNIT * 2);
	glVertex2f(0.f, -ARROW_UNIT);
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

static void
ClearScreen(const renderdata* prd)
{
	glClearColor(prd->BackGroundMat.ambientColour[0], prd->BackGroundMat.ambientColour[1],
		prd->BackGroundMat.ambientColour[2], 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

extern void
drawDie(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, int num)
{
	glPushMatrix();
	/* Move to correct position for die */
	if (bd3d->shakingDice) {
		glTranslatef(bd3d->diceMovingPos[num][0], bd3d->diceMovingPos[num][1], bd3d->diceMovingPos[num][2]);
		glRotatef(bd3d->diceRotation[num].xRot, 0.f, 1.f, 0.f);
		glRotatef(bd3d->diceRotation[num].yRot, 1.f, 0.f, 0.f);
		glRotatef(bd3d->dicePos[num][2], 0.f, 0.f, 1.f);
	}
	else
		moveToDicePos(bd, num);
	/* Now draw dice */
	drawDice(modelHolder, bd, num, bd->rd);

	glPopMatrix();
}

extern void
drawDice(const OglModelHolder* modelHolder, const BoardData* bd, int num, renderdata* prd)
{
	unsigned int value;
	int rotDice[6][2] = { {0, 0}, {0, 1}, {3, 0}, {1, 0}, {0, 3}, {2, 0} };
	int diceCol = (bd->turn == 1);
	int z;
	float diceSize = getDiceSize(bd->rd);
	diceTest dt;
	Material* pDiceMat = &bd->rd->DiceMat[diceCol];
	Material whiteMat;
	SetupSimpleMat(&whiteMat, 1.f, 1.f, 1.f);

	value = bd->diceRoll[num];

	/* During program startup value may be zero, if so don't draw */
	if (!value)
		return;
	value--;                    /* Zero based for array access */

	/* Get dice rotation */
	if (bd->diceShown == DICE_BELOW_BOARD)
		z = 0;
	else
		z = ((int)bd->bd3d->dicePos[num][2] + 45) / 90;

	/* Orientate dice correctly */
	glRotatef(90.0f * rotDice[value][0], 1.f, 0.f, 0.f);
	glRotatef(90.0f * rotDice[value][1], 0.f, 1.f, 0.f);

	/* DT = dice test, use to work out which way up the dice is */
	initDT(&dt, rotDice[value][0], rotDice[value][1], z);

	if (pDiceMat->alphaBlend) { /* Draw back of dice separately */
		glCullFace(GL_FRONT);
		glEnable(GL_BLEND);

		/* Draw dice */
		setMaterial(pDiceMat);
		OglModelDraw(&modelHolder->dice);

		/* Place back dots inside dice */
		setMaterial(&bd->rd->DiceDotMat[diceCol]);
		glEnable(GL_BLEND);     /* NB. Disabled in diceList */
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		drawDots(bd->bd3d, diceSize, -LIFT_OFF, &dt, 0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);
		// NB. GL_BLEND still enabled here (so front blends with back)
	}
	/* Draw dice */
	setMaterial(pDiceMat);
	OglModelDraw(&modelHolder->dice);

	/* Anti-alias dice edges */
	glLineWidth(1.f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	float size = getDiceSize(prd);
	float radius = size / 2.0f;
	int c;

	for (c = 0; c < 6; c++) {
		circleOutline(radius, radius + LIFT_OFF, prd->curveAccuracy);

		if (c % 2 == 0)
			glRotatef(-90.f, 0.f, 1.f, 0.f);
		else
			glRotatef(90.f, 1.f, 0.f, 0.f);
	}
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);

	/* Draw (front) dots */
	glEnable(GL_BLEND);
	/* First blank out space for dots */
	setMaterial(&whiteMat);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	drawDots(bd->bd3d, diceSize, LIFT_OFF, &dt, 1);

	/* Now fill space with coloured dots */
	setMaterial(&bd->rd->DiceDotMat[diceCol]);
	glBlendFunc(GL_ONE, GL_ONE);
	drawDots(bd->bd3d, diceSize, LIFT_OFF, &dt, 1);

	/* Restore blending defaults */
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void renderPiece(const OglModelHolder* modelHolder, int separateTop)
{
	if (separateTop)
	{
		glEnable(GL_TEXTURE_2D);
		OglModelDraw(&modelHolder->pieceTop);
		glDisable(GL_TEXTURE_2D);
	}

	OglModelDraw(&modelHolder->piece);
}

static void
renderSpecialPieces(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd)
{
	int separateTop = (prd->ChequerMat[0].pTexture && prd->pieceTextureType == PTT_TOP);

	if (bd->drag_point >= 0) {
		glPushMatrix();
		glTranslatef(bd3d->dragPos[0], bd3d->dragPos[1], bd3d->dragPos[2]);
		glRotatef((float)bd3d->movingPieceRotation, 0.f, 0.f, 1.f);
		setMaterial(&prd->ChequerMat[(bd->drag_colour == 1) ? 1 : 0]);
		renderPiece(modelHolder, separateTop);
		glPopMatrix();
	}

	if (bd3d->moving) {
		glPushMatrix();
		glTranslatef(bd3d->movingPos[0], bd3d->movingPos[1], bd3d->movingPos[2]);
		if (bd3d->rotateMovingPiece > 0)
			glRotatef(-90 * bd3d->rotateMovingPiece * bd->turn, 1.f, 0.f, 0.f);
		glRotatef((float)bd3d->movingPieceRotation, 0.f, 0.f, 1.f);
		setMaterial(&prd->ChequerMat[(bd->turn == 1) ? 1 : 0]);
		renderPiece(modelHolder, separateTop);
		glPopMatrix();
	}
}

static void
drawSpecialPieces(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd)
{                               /* Draw animated or dragged pieces */
	int blend = (prd->ChequerMat[0].alphaBlend) || (prd->ChequerMat[1].alphaBlend);

	if (blend) {                /* Draw back of piece separately */
		glCullFace(GL_FRONT);
		glEnable(GL_BLEND);
		renderSpecialPieces(modelHolder, bd, bd3d, prd);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
	}
	renderSpecialPieces(modelHolder, bd, bd3d, prd);

	if (blend)
		glDisable(GL_BLEND);
}

extern void
drawPiece(const OglModelHolder* modelHolder, const BoardData3d* bd3d, unsigned int point, unsigned int pos, int rotate, int roundPiece, int curveAccuracy, int separateTop)
{
	float v[3];
	glPushMatrix();

	getPiecePos(point, pos, v);
	glTranslatef(v[0], v[1], v[2]);

	/* Home pieces are sideways */
	if (point == 26)
		glRotatef(-90.f, 1.f, 0.f, 0.f);
	if (point == 27)
		glRotatef(90.f, 1.f, 0.f, 0.f);

	if (rotate)
		glRotatef((float)bd3d->pieceRotation[point][pos - 1], 0.f, 0.f, 1.f);

	renderPiece(modelHolder, separateTop);

	/* Anti-alias piece edges */
	float lip = 0;
	float radius = PIECE_HOLE / 2.0f;
	float discradius = radius * 0.8f;

	if (roundPiece)
		lip = radius - discradius;

	GLboolean textureEnabled;
	glGetBooleanv(GL_TEXTURE_2D, &textureEnabled);

	if (textureEnabled)
		glDisable(GL_TEXTURE_2D);

	glLineWidth(1.f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	circleOutlineOutward(radius, PIECE_DEPTH - lip, curveAccuracy);
	circleOutlineOutward(radius, lip, curveAccuracy);

	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glDepthMask(GL_TRUE);

	if (textureEnabled)
		glEnable(GL_TEXTURE_2D);

	glPopMatrix();
}

extern void
drawPieces(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd)
{
	unsigned int i;
	unsigned int j;
	int blend = (prd->ChequerMat[0].alphaBlend) || (prd->ChequerMat[1].alphaBlend);
	int separateTop = (prd->ChequerMat[0].pTexture && prd->pieceTextureType == PTT_TOP);

	if (blend) {                /* Draw back of piece separately */
		glCullFace(GL_FRONT);

		setMaterial(&prd->ChequerMat[0]);
		for (i = 0; i < 28; i++) {
			if (bd->points[i] < 0) {
				unsigned int numPieces = (unsigned int)(-bd->points[i]);
				for (j = 1; j <= numPieces; j++) {
					glEnable(GL_BLEND);
					drawPiece(modelHolder, bd3d, i, j, TRUE, (prd->pieceType == PT_ROUNDED), prd->curveAccuracy, separateTop);
				}
			}
		}
		setMaterial(&prd->ChequerMat[1]);
		for (i = 0; i < 28; i++) {
			if (bd->points[i] > 0) {
				unsigned int numPieces = (unsigned int)bd->points[i];
				for (j = 1; j <= numPieces; j++) {
					glEnable(GL_BLEND);
					drawPiece(modelHolder, bd3d, i, j, TRUE, (prd->pieceType == PT_ROUNDED), prd->curveAccuracy, separateTop);
				}
			}
		}
		glCullFace(GL_BACK);
	}

	setMaterial(&prd->ChequerMat[0]);
	for (i = 0; i < 28; i++) {
		if (bd->points[i] < 0) {
			unsigned int numPieces = (unsigned int)(-bd->points[i]);
			for (j = 1; j <= numPieces; j++) {
				if (blend)
					glEnable(GL_BLEND);
				drawPiece(modelHolder, bd3d, i, j, TRUE, (prd->pieceType == PT_ROUNDED), prd->curveAccuracy, separateTop);
			}
		}
	}
	setMaterial(&prd->ChequerMat[1]);
	for (i = 0; i < 28; i++) {
		if (bd->points[i] > 0) {
			unsigned int numPieces = (unsigned int)bd->points[i];
			for (j = 1; j <= numPieces; j++) {
				if (blend)
					glEnable(GL_BLEND);
				drawPiece(modelHolder, bd3d, i, j, TRUE, (prd->pieceType == PT_ROUNDED), prd->curveAccuracy, separateTop);
			}
		}
	}
	if (blend)
		glDisable(GL_BLEND);

	if (bd->DragTargetHelp) {   /* highlight target points */
		glPolygonMode(GL_FRONT, GL_LINE);
		SetColour3d(0.f, 1.f, 0.f, 0.f);        /* Nice bright green... */

		for (i = 0; i <= 3; i++) {
			int target = bd->iTargetHelpPoints[i];
			if (target != -1) { /* Make sure texturing is disabled */
				if (prd->ChequerMat[0].pTexture)
					glDisable(GL_TEXTURE_2D);
				drawPiece(modelHolder, bd3d, (unsigned int)target, Abs(bd->points[target]) + 1, TRUE, (prd->pieceType == PT_ROUNDED), prd->curveAccuracy, separateTop);
			}
		}
		glPolygonMode(GL_FRONT, GL_FILL);
	}
}

extern void
drawPointLegacy(const renderdata* prd, float tuv, unsigned int i, int p, int outline)
{                               /* Draw point with correct texture co-ords */
	float w = PIECE_HOLE;
	float h = POINT_HEIGHT;
	float x, y;

	if (p) {
		x = TRAY_WIDTH - EDGE_WIDTH + PIECE_HOLE * i;
		y = -LIFT_OFF;
	}
	else {
		x = TRAY_WIDTH - EDGE_WIDTH + BOARD_WIDTH - (PIECE_HOLE * i);
		y = TOTAL_HEIGHT - EDGE_HEIGHT * 2 + LIFT_OFF;
		w = -w;
		h = -h;
	}

	glPushMatrix();
	if (prd->bgInTrays)
		glTranslatef(EDGE_WIDTH, EDGE_HEIGHT, BASE_DEPTH);
	else {
		x -= TRAY_WIDTH - EDGE_WIDTH;
		glTranslatef(TRAY_WIDTH, EDGE_HEIGHT, BASE_DEPTH);
	}

	if (prd->roundedPoints) {   /* Draw rounded point ends */
		float xoff;

		w = w * TAKI_WIDTH;
		y += w / 2.0f;
		h -= w / 2.0f;

		if (p)
			xoff = x + (PIECE_HOLE / 2.0f);
		else
			xoff = x - (PIECE_HOLE / 2.0f);

		/* Draw rounded semi-circle end of point (with correct texture co-ords) */
		{
			unsigned int j;
			float angle, step;
			float radius = w / 2.0f;

			step = (2 * F_PI) / prd->curveAccuracy;
			angle = -step * (int)(prd->curveAccuracy / 4);
			glNormal3f(0.f, 0.f, 1.f);
			glBegin(outline ? GL_LINE_STRIP : GL_TRIANGLE_FAN);
			glTexCoord2f(xoff * tuv, y * tuv);

			glVertex3f(xoff, y, 0.f);
			for (j = 0; j <= prd->curveAccuracy / 2; j++) {
				glTexCoord2f((xoff + sinf(angle) * radius) * tuv, (y + cosf(angle) * radius) * tuv);
				glVertex3f(xoff + sinf(angle) * radius, y + cosf(angle) * radius, 0.f);
				angle -= step;
			}
			glEnd();
		}
		/* Move rest of point in slighlty */
		if (p)
			x -= -((PIECE_HOLE * (1 - TAKI_WIDTH)) / 2.0f);
		else
			x -= ((PIECE_HOLE * (1 - TAKI_WIDTH)) / 2.0f);
	}

	glBegin(outline ? GL_LINE_STRIP : GL_TRIANGLES);
	glNormal3f(0.f, 0.f, 1.f);
	glTexCoord2f((x + w) * tuv, y * tuv);
	glVertex3f(x + w, y, 0.f);
	glTexCoord2f((x + w / 2) * tuv, (y + h) * tuv);
	glVertex3f(x + w / 2, y + h, 0.f);
	glTexCoord2f(x * tuv, y * tuv);
	glVertex3f(x, y, 0.f);
	glEnd();

	glPopMatrix();
}

void MAApoints(const renderdata* prd)
{
	float tuv;

	setMaterial(&prd->PointMat[0]);

	if (prd->PointMat[0].pTexture)
		tuv = (TEXTURE_SCALE) / prd->PointMat[0].pTexture->width;
	else
		tuv = 0;

	glLineWidth(1.f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);

	drawPointLegacy(prd, tuv, 0, 0, 1);
	drawPointLegacy(prd, tuv, 0, 1, 1);
	drawPointLegacy(prd, tuv, 2, 0, 1);
	drawPointLegacy(prd, tuv, 2, 1, 1);
	drawPointLegacy(prd, tuv, 4, 0, 1);
	drawPointLegacy(prd, tuv, 4, 1, 1);

	setMaterial(&prd->PointMat[1]);

	if (prd->PointMat[1].pTexture)
		tuv = (TEXTURE_SCALE) / prd->PointMat[1].pTexture->width;
	else
		tuv = 0;

	drawPointLegacy(prd, tuv, 1, 0, 1);
	drawPointLegacy(prd, tuv, 1, 1, 1);
	drawPointLegacy(prd, tuv, 3, 0, 1);
	drawPointLegacy(prd, tuv, 3, 1, 1);
	drawPointLegacy(prd, tuv, 5, 0, 1);
	drawPointLegacy(prd, tuv, 5, 1, 1);

	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
}

extern void
drawDC(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd)
{
	glPushMatrix();
	moveToDoubleCubePos(bd);

	setMaterial(&prd->CubeMat);
	OglModelDraw(&modelHolder->DC);

	DrawDCNumbers(bd);

	glPopMatrix();
}

void drawTable(const OglModelHolder* modelHolder, const BoardData3d* bd3d, const renderdata* prd)
{
	glClear(GL_DEPTH_BUFFER_BIT);	// Could just overwrite values with the board instead
	glDepthFunc(GL_ALWAYS);

	setMaterial(&prd->BackGroundMat);
	OglModelDraw(&modelHolder->background);

	/* Left hand side points */
	setMaterial(&prd->BaseMat);
	OglModelDraw(&modelHolder->boardBase);
	setMaterial(&prd->PointMat[0]);
	OglModelDraw(&modelHolder->oddPoints);
	setMaterial(&prd->PointMat[1]);
	OglModelDraw(&modelHolder->evenPoints);

	MAApoints(prd);

	/* Rotate around for right board */
	glPushMatrix();
	glTranslatef(TOTAL_WIDTH, TOTAL_HEIGHT, 0.f);
	glRotatef(180.f, 0.f, 0.f, 1.f);
	setMaterial(&prd->BaseMat);
	OglModelDraw(&modelHolder->boardBase);
	setMaterial(&prd->PointMat[0]);
	OglModelDraw(&modelHolder->oddPoints);
	setMaterial(&prd->PointMat[1]);
	OglModelDraw(&modelHolder->evenPoints);

	MAApoints(prd);
	glPopMatrix();

	glDepthFunc(GL_LEQUAL);

	setMaterial(&prd->BoxMat);
	OglModelDraw(&modelHolder->table);

	if (prd->fHinges3d)
	{
		setMaterial(&prd->HingeMat);
		glPushMatrix();
		glTranslatef((TOTAL_WIDTH) / 2.0f, ((TOTAL_HEIGHT / 2.0f) - HINGE_HEIGHT) / 2.0f, BASE_DEPTH + EDGE_DEPTH);
		OglModelDraw(&modelHolder->hinge);
		glPopMatrix();
		glPushMatrix();
		glTranslatef((TOTAL_WIDTH) / 2.0f, ((TOTAL_HEIGHT / 2.0f) - HINGE_HEIGHT + TOTAL_HEIGHT) / 2.0f, BASE_DEPTH + EDGE_DEPTH);
		OglModelDraw(&modelHolder->hinge);
		glPopMatrix();

		/* Shadow in gap between boards */
		setMaterial(&bd3d->gapColour);
		drawRect((TOTAL_WIDTH - HINGE_GAP * 1.5f) / 2.0f, 0.f, LIFT_OFF, HINGE_GAP * 2, TOTAL_HEIGHT + LIFT_OFF, 0);
	}
}

static void
getProjectedPos(int x, int y, float atDepth, float pos[3])
{                               /* Work out where point (x, y) projects to at depth atDepth */
	int viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	double nearX, nearY, nearZ, farX, farY, farZ, zRange;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	if ((gluUnProject((GLdouble)x, (GLdouble)y, 0.0, mvmatrix, projmatrix, viewport, &nearX, &nearY, &nearZ) ==
		GL_FALSE)
		|| (gluUnProject((GLdouble)x, (GLdouble)y, 1.0, mvmatrix, projmatrix, viewport, &farX, &farY, &farZ) ==
			GL_FALSE)) {
		/* Maybe a g_assert_not_reached() would be appropriate here
		 * Don't leave output parameters undefined anyway */
		pos[0] = pos[1] = pos[2] = 0.0f;
		g_print("gluUnProject failed!\n");
		return;
	}

	zRange = (fabs(nearZ) - atDepth) / (fabs(farZ) + fabs(nearZ));
	pos[0] = (float)(nearX - (-farX + nearX) * zRange);
	pos[1] = (float)(nearY - (-farY + nearY) * zRange);
	pos[2] = (float)(nearZ - (-farZ + nearZ) * zRange);
}

void
getProjectedPieceDragPos(int x, int y, float pos[3])
{
	getProjectedPos(x, y, BASE_DEPTH + EDGE_DEPTH + DOUBLECUBE_SIZE + LIFT_OFF * 3, pos);
}

void
calculateBackgroundSize(BoardData3d* bd3d, const int viewport[4])
{
	float pos1[3], pos2[3], pos3[3];

	getProjectedPos(0, viewport[1] + viewport[3], 0.f, pos1);
	getProjectedPos(viewport[2], viewport[1] + viewport[3], 0.f, pos2);
	getProjectedPos(0, viewport[1], 0.f, pos3);

	bd3d->backGroundPos[0] = pos1[0];
	bd3d->backGroundPos[1] = pos3[1];
	bd3d->backGroundSize[0] = pos2[0] - pos1[0];
	bd3d->backGroundSize[1] = pos1[1] - pos3[1];
}

void
SetupPerspVolume(const BoardData* bd, BoardData3d* bd3d, const renderdata* prd, int viewport[4])
{
	float aspectRatio = (float)viewport[2] / (float)(viewport[3]);
	if (!prd->planView) {
		viewArea va;
		float halfRadianFOV;
		double fovScale;
		float zoom;

		if (aspectRatio < .5f) {        /* Viewing area to high - cut down so board rendered correctly */
			int newHeight = viewport[2] * 2;
			viewport[1] = (viewport[3] - newHeight) / 2;
			viewport[3] = newHeight;
			glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
			aspectRatio = .5f;
			/* Clear screen so top + bottom outside board shown ok */
			ClearScreen(prd);
		}

		/* Workout how big the board is (in 3d space) */
		WorkOutViewArea(bd, &va, &halfRadianFOV, aspectRatio);

		fovScale = zNear * tanf(halfRadianFOV);

		if (aspectRatio > getAreaRatio(&va)) {
			bd3d->vertFrustrum = fovScale;
			bd3d->horFrustrum = bd3d->vertFrustrum * aspectRatio;
		}
		else {
			bd3d->horFrustrum = fovScale * getAreaRatio(&va);
			bd3d->vertFrustrum = bd3d->horFrustrum / aspectRatio;
		}
		/* Setup projection matrix */
		glFrustum(-bd3d->horFrustrum, bd3d->horFrustrum, -bd3d->vertFrustrum, bd3d->vertFrustrum, zNear, zFar);

		/* Setup modelview matrix */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		/* Zoom back so image fills window */
		zoom = (getViewAreaHeight(&va) / 2) / tanf(halfRadianFOV);
		glTranslatef(0.f, 0.f, -zoom);

		/* Offset from centre because of perspective */
		glTranslatef(0.f, getViewAreaHeight(&va) / 2 + va.bottom, 0.f);

		/* Rotate board */
		glRotatef((float)prd->boardAngle, -1.f, 0.f, 0.f);

		/* Origin is bottom left, so move from centre */
		glTranslatef(-(getBoardWidth() / 2.0f), -((getBoardHeight()) / 2.0f), 0.f);
	}
	else {
		float size;

		if (aspectRatio > getBoardWidth() / getBoardHeight()) {
			size = (getBoardHeight() / 2);
			bd3d->horFrustrum = size * aspectRatio;
			bd3d->vertFrustrum = size;
		}
		else {
			size = (getBoardWidth() / 2);
			bd3d->horFrustrum = size;
			bd3d->vertFrustrum = size / aspectRatio;
		}
		glOrtho(-bd3d->horFrustrum, bd3d->horFrustrum, -bd3d->vertFrustrum, bd3d->vertFrustrum, 0.0, 5.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glTranslatef(-(getBoardWidth() / 2.0f), -(getBoardHeight() / 2.0f), -3.f);
	}

	/* Save matrix for later */
	glGetFloatv(GL_MODELVIEW_MATRIX, bd3d->modelMatrix);
}

extern void
renderFlag(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, unsigned int curveAccuracy)
{
	/* Simple knots i.e. no weighting */
	float s_knots[S_NUMKNOTS] = { 0, 0, 0, 0, 1, 1, 1, 1 };
	float t_knots[T_NUMKNOTS] = { 0, 0, 1, 1 };

	/* Draw flag surface */
	setMaterial(&bd3d->flagMat);

	if (flag.flagNurb == NULL)
		flag.flagNurb = gluNewNurbsRenderer();

	/* Set size of polygons */
	gluNurbsProperty(flag.flagNurb, GLU_SAMPLING_TOLERANCE, 500.0f / curveAccuracy);

	gluBeginSurface(flag.flagNurb);
	gluNurbsSurface(flag.flagNurb, S_NUMKNOTS, s_knots, T_NUMKNOTS, t_knots, 3 * T_NUMPOINTS, 3,
		&flag.ctlpoints[0][0][0], S_NUMPOINTS, T_NUMPOINTS, GL_MAP2_VERTEX_3);
	gluEndSurface(flag.flagNurb);

	/* Draw flag pole */
	SetColour3d(.2f, .2f, .4f, 0.f);    /* Blue pole */
	OglModelDraw(&modelHolder->flagPole);

	/* Draw number on flag */
	glDisable(GL_DEPTH_TEST);

	setMaterial(&bd3d->flagNumberMat);

	glPushMatrix();
	{
		/* Move to middle of flag */
		float v[3];
		v[0] = (flag.ctlpoints[1][0][0] + flag.ctlpoints[2][0][0]) / 2.0f;
		v[1] = (flag.ctlpoints[1][0][0] + flag.ctlpoints[1][1][0]) / 2.0f;
		v[2] = (flag.ctlpoints[1][0][2] + flag.ctlpoints[2][0][2]) / 2.0f;
		glTranslatef(v[0], v[1], v[2]);
	}
	{
		/* Work out approx angle of number based on control points */
		float ang =
			atanf(-(flag.ctlpoints[2][0][2] - flag.ctlpoints[1][0][2]) /
			(flag.ctlpoints[2][0][0] - flag.ctlpoints[1][0][0]));
		float degAng = (ang) * 180 / F_PI;

		glRotatef(degAng, 0.f, 1.f, 0.f);
	}

	{
		/* Draw number */
		char flagValue[2] = "x";
		/* No specular light */
		float specular[4];
		float zero[4] = { 0, 0, 0, 0 };
		glGetLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		glLightfv(GL_LIGHT0, GL_SPECULAR, zero);

		flagValue[0] = '0' + (char)abs(bd->resigned);
		glScalef(1.3f, 1.3f, 1.f);

		glLineWidth(.5f);
		glPrintCube(&bd3d->cubeFont, flagValue, /*MAA*/0);

		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	}
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
}

static void
drawFlag(const OglModelHolder* modelHolder, const BoardData* bd, const BoardData3d* bd3d, const renderdata* prd)
{
	float v[4];
	int isStencil = glIsEnabled(GL_STENCIL_TEST);

	if (isStencil)
		glDisable(GL_STENCIL_TEST);

	waveFlag(bd3d->flagWaved);

	glPushMatrix();

	getFlagPos(bd, v);
	glTranslatef(v[0], v[1], v[2]);

	renderFlag(modelHolder, bd, bd3d, prd->curveAccuracy);

	glPopMatrix();
	if (isStencil)
		glEnable(GL_STENCIL_TEST);
}
