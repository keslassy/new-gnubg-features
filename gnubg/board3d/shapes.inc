
static void
drawRect(float x, float y, float z, float w, float h, const Texture* texture)
{                               /* Draw a rectangle */
	glPushMatrix();

	glTranslatef(x + w / 2, y + h / 2, z);
	glScalef(w / 2.0f, h / 2.0f, 1.f);
	glNormal3f(0.f, 0.f, 1.f);

	if (texture) {
		float tuv = TEXTURE_SCALE / texture->width;
		float repX = w * tuv;
		float repY = h * tuv;

		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f);
		glVertex3f(-1.f, -1.f, 0.f);
		glTexCoord2f(repX, 0.f);
		glVertex3f(1.f, -1.f, 0.f);
		glTexCoord2f(repX, repY);
		glVertex3f(1.f, 1.f, 0.f);
		glTexCoord2f(0.f, repY);
		glVertex3f(-1.f, 1.f, 0.f);
		glEnd();
    } else {
		glBegin(GL_QUADS);
		glVertex3f(-1.f, -1.f, 0.f);
		glVertex3f(1.f, -1.f, 0.f);
		glVertex3f(1.f, 1.f, 0.f);
		glVertex3f(-1.f, 1.f, 0.f);
		glEnd();
	}

	glPopMatrix();
}

static void
drawSplitRect(float x, float y, float z, float w, float h, const Texture* texture)
{                               /* Draw a rectangle in 2 bits */
	glPushMatrix();

	glTranslatef(x + w / 2, y + h / 2, z);
	glScalef(w / 2.0f, h / 2.0f, 1.f);
	glNormal3f(0.f, 0.f, 1.f);

	if (texture) {
		float tuv = TEXTURE_SCALE / texture->width;
		float repX = w * tuv;
		float repY = h * tuv;

		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f);
		glVertex3f(-1.f, -1.f, 0.f);
		glTexCoord2f(repX, 0.f);
		glVertex3f(1.f, -1.f, 0.f);
		glTexCoord2f(repX, repY / 2.0f);
		glVertex3f(1.f, 0.f, 0.f);
		glTexCoord2f(0.f, repY / 2.0f);
		glVertex3f(-1.f, 0.f, 0.f);

		glTexCoord2f(0.f, repY / 2.0f);
		glVertex3f(-1.f, 0.f, 0.f);
		glTexCoord2f(repX, repY / 2.0f);
		glVertex3f(1.f, 0.f, 0.f);
		glTexCoord2f(repX, repY);
		glVertex3f(1.f, 1.f, 0.f);
		glTexCoord2f(0.f, repY);
		glVertex3f(-1.f, 1.f, 0.f);
		glEnd();
    } else {
		glBegin(GL_QUADS);
		glVertex3f(-1.f, -1.f, 0.f);
		glVertex3f(1.f, -1.f, 0.f);
		glVertex3f(1.f, 0.f, 0.f);
		glVertex3f(-1.f, 0.f, 0.f);

		glVertex3f(-1.f, 0.f, 0.f);
		glVertex3f(1.f, 0.f, 0.f);
		glVertex3f(1.f, 1.f, 0.f);
		glVertex3f(-1.f, 1.f, 0.f);
		glEnd();
	}

	glPopMatrix();
}

static void
drawChequeredRect(float x, float y, float z, float w, float h, int across, int down, const Texture* texture)
{                               /* Draw a rectangle split into (across x down) chequers */
	int i, j;
	float hh = h / down;
	float ww = w / across;

	glPushMatrix();
	glTranslatef(x, y, z);
	glNormal3f(0.f, 0.f, 1.f);

	if (texture) {
		float tuv = TEXTURE_SCALE / texture->width;
		float tw = ww * tuv;
		float th = hh * tuv;
		float ty = 0.f;

		for (i = 0; i < down; i++) {
			float xx = 0, tx = 0;
			glPushMatrix();
			glTranslatef(0.f, hh * i, 0.f);
			glBegin(GL_QUAD_STRIP);
			for (j = 0; j <= across; j++) {
				glTexCoord2f(tx, ty + th);
				glVertex2f(xx, hh);
				glTexCoord2f(tx, ty);
				glVertex2f(xx, 0.f);
				xx += ww;
				tx += tw;
			}
			ty += th;
			glEnd();
			glPopMatrix();
		}
    } else {
		for (i = 0; i < down; i++) {
			float xx = 0;
			glPushMatrix();
			glTranslatef(0.f, hh * i, 0.f);
			glBegin(GL_QUAD_STRIP);
			for (j = 0; j <= across; j++) {
				glVertex2f(xx, hh);
				glVertex2f(xx, 0.f);
				xx += ww;
			}
			glEnd();
			glPopMatrix();
		}
	}
	glPopMatrix();
}

static void
QuarterCylinder(float radius, float len, unsigned int accuracy, const Texture* texture)
{
	unsigned int i;
	float d;
	float dInc = 0;

	/* texture unit value */
	float tuv;
	if (texture) {
		float st = sinf((2 * F_PI) / accuracy) * radius;
		float ct = (cosf((2 * F_PI) / accuracy) - 1) * radius;
		dInc = sqrtf(st * st + ct * ct);
		tuv = (TEXTURE_SCALE) / texture->width;
    } else
		tuv = 0.0f;

	d = 0;
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < accuracy / 4 + 1; i++) {
		float sar, car;
		float angle = ((float)i * 2.f * F_PI) / accuracy;

		glNormal3f(sinf(angle), 0.f, cosf(angle));

		sar = sinf(angle) * radius;
		car = cosf(angle) * radius;

		if (tuv != 0.0f)
			glTexCoord2f(len * tuv, d * tuv);
		glVertex3f(sar, len, car);

		if (tuv != 0.0f) {
			glTexCoord2f(0.f, d * tuv);
			d -= dInc;
		}
		glVertex3f(sar, 0.f, car);
	}
	glEnd();
}

static void
QuarterCylinderSplayedRev(float radius, float len, unsigned int accuracy, const Texture* texture)
{
	unsigned int i;
	float d;
	float dInc = 0;

	/* texture unit value */
	float tuv;
	if (texture) {
		float st = sinf((2 * F_PI) / accuracy) * radius;
		float ct = (cosf((2 * F_PI) / accuracy) - 1) * radius;
		dInc = sqrtf(st * st + ct * ct);
		tuv = (TEXTURE_SCALE) / texture->width;
    } else
		tuv = 0;

	d = 0;
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < accuracy / 4 + 1; i++) {
		float sar, car;
		float angle = ((float)i * 2.f * F_PI) / accuracy;

		glNormal3f(sinf(angle), 0.f, cosf(angle));

		sar = sinf(angle) * radius;
		car = cosf(angle) * radius;

		if (tuv != 0.0f)
			glTexCoord2f((len + car) * tuv, d * tuv);
		glVertex3f(sar, len + car, car);

		if (tuv != 0.0f) {
			glTexCoord2f(-car * tuv, d * tuv);
			d -= dInc;
		}
		glVertex3f(sar, -car, car);
	}
	glEnd();
}

static void
QuarterCylinderSplayed(float radius, float len, unsigned int accuracy, const Texture* texture)
{
	unsigned int i;
	float d;
	float dInc = 0;

	/* texture unit value */
	float tuv;
	if (texture) {
		float st = sinf((2 * F_PI) / accuracy) * radius;
		float ct = (cosf((2 * F_PI) / accuracy) - 1) * radius;
		dInc = sqrtf(st * st + ct * ct);
		tuv = (TEXTURE_SCALE) / texture->width;
    } else
		tuv = 0;

	d = 0;
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < accuracy / 4 + 1; i++) {
		float sar, car;
		float angle = ((float)i * 2.f * F_PI) / accuracy;

		glNormal3f(sinf(angle), 0.f, cosf(angle));

		sar = sinf(angle) * radius;
		car = cosf(angle) * radius;

		if (tuv != 0.0f)
			glTexCoord2f((len - car) * tuv, d * tuv);
		glVertex3f(sar, len - car, car);

		if (tuv != 0.0f) {
			glTexCoord2f(car * tuv, d * tuv);
			d -= dInc;
		}
		glVertex3f(sar, car, car);
	}
	glEnd();
}

static void
drawCornerEigth(const EigthPoints* eigthPoints, float radius, const Texture* texture)
{
	unsigned int i;
	int j;

	/* texture unit value */
	float tuv;
	if (texture)
		tuv = (TEXTURE_SCALE) / texture->width;
	else
		tuv = 0.0f;

	for (i = 0; i < eigthPoints->accuracy / 4; i++) {
		unsigned int ns = (eigthPoints->accuracy / 4) - (i + 1);

		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(eigthPoints->points[i][ns + 1][0] / radius, eigthPoints->points[i][ns + 1][1] / radius,
			eigthPoints->points[i][ns + 1][2] / radius);
		if (tuv != 0.0f)
			glTexCoord2f(eigthPoints->points[i][ns + 1][0] * tuv, eigthPoints->points[i][ns + 1][1] * tuv);	// Here and below - approximate texture co-ords
		glVertex3f(eigthPoints->points[i][ns + 1][0], eigthPoints->points[i][ns + 1][1], eigthPoints->points[i][ns + 1][2]);
		for (j = (int)ns; j >= 0; j--) {
			glNormal3f(eigthPoints->points[i + 1][j][0] / radius, eigthPoints->points[i + 1][j][1] / radius,
				eigthPoints->points[i + 1][j][2] / radius);
			if (tuv != 0.0f)
				glTexCoord2f(eigthPoints->points[i + 1][j][0] * tuv, eigthPoints->points[i + 1][j][1] * tuv);
			glVertex3f(eigthPoints->points[i + 1][j][0], eigthPoints->points[i + 1][j][1], eigthPoints->points[i + 1][j][2]);
			glNormal3f(eigthPoints->points[i][j][0] / radius, eigthPoints->points[i][j][1] / radius, eigthPoints->points[i][j][2] / radius);
			if (tuv != 0.0f)
				glTexCoord2f(eigthPoints->points[i][j][0] * tuv, eigthPoints->points[i][j][1] * tuv);
			glVertex3f(eigthPoints->points[i][j][0], eigthPoints->points[i][j][1], eigthPoints->points[i][j][2]);
		}
		glEnd();
	}
}

static void
drawBox(int type, float x, float y, float z, float w, float h, float d, const Texture* texture)
{                               /* Draw a box with normals and optional textures */
	float normX, normY, normZ;
	float w2 = w / 2.0f, h2 = h / 2.0f, d2 = d / 2.0f;

	glPushMatrix();
	glTranslatef(x + w2, y + h2, z + d2);
	glScalef(w2, h2, d2);

	/* Scale normals */
	normX = w2;
	normY = h2;
	normZ = d2;

	glBegin(GL_QUADS);

	if (texture) {
		float repX = (w * TEXTURE_SCALE) / texture->width;
		float repY = (h * TEXTURE_SCALE) / texture->height;

		/* Front Face */
		glNormal3f(0.f, 0.f, normZ);
		if (type & BOX_SPLITTOP) {
			glTexCoord2f(0.f, 0.f);
			glVertex3f(-1.f, -1.f, 1.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(1.f, -1.f, 1.f);
			glTexCoord2f(repX, repY / 2.0f);
			glVertex3f(1.f, 0.f, 1.f);
			glTexCoord2f(0.f, repY / 2.0f);
			glVertex3f(-1.f, 0.f, 1.f);

			glTexCoord2f(0.f, repY / 2.0f);
			glVertex3f(-1.f, 0.f, 1.f);
			glTexCoord2f(repX, repY / 2.0f);
			glVertex3f(1.f, 0.f, 1.f);
			glTexCoord2f(repX, repY);
			glVertex3f(1.f, 1.f, 1.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(-1.f, 1.f, 1.f);
        } else if (type & BOX_SPLITWIDTH) {
			glTexCoord2f(0.f, 0.f);
			glVertex3f(-1.f, -1.f, 1.f);
			glTexCoord2f(repX / 2.0f, 0.f);
			glVertex3f(0.f, -1.f, 1.f);
			glTexCoord2f(repX / 2.0f, repY);
			glVertex3f(0.f, 1.f, 1.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(-1.f, 1.f, 1.f);

			glTexCoord2f(repX / 2.0f, 0.f);
			glVertex3f(0.f, -1.f, 1.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(1.f, -1.f, 1.f);
			glTexCoord2f(repX, repY);
			glVertex3f(1.f, 1.f, 1.f);
			glTexCoord2f(repX / 2.0f, repY);
			glVertex3f(0.f, 1.f, 1.f);
        } else {
			glTexCoord2f(0.f, 0.f);
			glVertex3f(-1.f, -1.f, 1.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(1.f, -1.f, 1.f);
			glTexCoord2f(repX, repY);
			glVertex3f(1.f, 1.f, 1.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(-1.f, 1.f, 1.f);
		}
		if (!(type & BOX_NOENDS)) {
			/* Top Face */
			glNormal3f(0.f, normY, 0.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(-1.f, 1.f, -1.f);
			glTexCoord2f(0.f, 0.f);
			glVertex3f(-1.f, 1.f, 1.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(1.f, 1.f, 1.f);
			glTexCoord2f(repX, repY);
			glVertex3f(1.f, 1.f, -1.f);
			/* Bottom Face */
			glNormal3f(0.f, -normY, 0.f);
			glTexCoord2f(repX, repY);
			glVertex3f(-1.f, -1.f, -1.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(1.f, -1.f, -1.f);
			glTexCoord2f(0.f, 0.f);
			glVertex3f(1.f, -1.f, 1.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(-1.f, -1.f, 1.f);
		}
		if (!(type & BOX_NOSIDES)) {
			/* Right face */
			glNormal3f(normX, 0.f, 0.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(1.f, -1.f, -1.f);
			glTexCoord2f(repX, repY);
			glVertex3f(1.f, 1.f, -1.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(1.f, 1.f, 1.f);
			glTexCoord2f(0.f, 0.f);
			glVertex3f(1.f, -1.f, 1.f);
			/* Left Face */
			glNormal3f(-normX, 0.f, 0.f);
			glTexCoord2f(0.f, 0.f);
			glVertex3f(-1.f, -1.f, -1.f);
			glTexCoord2f(repX, 0.f);
			glVertex3f(-1.f, -1.f, 1.f);
			glTexCoord2f(repX, repY);
			glVertex3f(-1.f, 1.f, 1.f);
			glTexCoord2f(0.f, repY);
			glVertex3f(-1.f, 1.f, -1.f);
		}
    } else {                    /* no texture co-ords */
	 /* Front Face */
		glNormal3f(0.f, 0.f, normZ);
		if (type & BOX_SPLITTOP) {
			glVertex3f(-1.f, -1.f, 1.f);
			glVertex3f(1.f, -1.f, 1.f);
			glVertex3f(1.f, 0.f, 1.f);
			glVertex3f(-1.f, 0.f, 1.f);

			glVertex3f(-1.f, 0.f, 1.f);
			glVertex3f(1.f, 0.f, 1.f);
			glVertex3f(1.f, 1.f, 1.f);
			glVertex3f(-1.f, 1.f, 1.f);
        } else if (type & BOX_SPLITWIDTH) {
			glVertex3f(-1.f, -1.f, 1.f);
			glVertex3f(0.f, -1.f, 1.f);
			glVertex3f(0.f, 1.f, 1.f);
			glVertex3f(-1.f, 1.f, 1.f);

			glVertex3f(0.f, -1.f, 1.f);
			glVertex3f(1.f, -1.f, 1.f);
			glVertex3f(1.f, 1.f, 1.f);
			glVertex3f(0.f, 1.f, 1.f);
        } else {
			glVertex3f(-1.f, -1.f, 1.f);
			glVertex3f(1.f, -1.f, 1.f);
			glVertex3f(1.f, 1.f, 1.f);
			glVertex3f(-1.f, 1.f, 1.f);
		}

		if (!(type & BOX_NOENDS)) {
			/* Top Face */
			glNormal3f(0.f, normY, 0.f);
			glVertex3f(-1.f, 1.f, -1.f);
			glVertex3f(-1.f, 1.f, 1.f);
			glVertex3f(1.f, 1.f, 1.f);
			glVertex3f(1.f, 1.f, -1.f);
			/* Bottom Face */
			glNormal3f(0.f, -normY, 0.f);
			glVertex3f(-1.f, -1.f, -1.f);
			glVertex3f(1.f, -1.f, -1.f);
			glVertex3f(1.f, -1.f, 1.f);
			glVertex3f(-1.f, -1.f, 1.f);
		}
		if (!(type & BOX_NOSIDES)) {
			/* Right face */
			glNormal3f(normX, 0.f, 0.f);
			glVertex3f(1.f, -1.f, -1.f);
			glVertex3f(1.f, 1.f, -1.f);
			glVertex3f(1.f, 1.f, 1.f);
			glVertex3f(1.f, -1.f, 1.f);
			/* Left Face */
			glNormal3f(-normX, 0.f, 0.f);
			glVertex3f(-1.f, -1.f, -1.f);
			glVertex3f(-1.f, -1.f, 1.f);
			glVertex3f(-1.f, 1.f, 1.f);
			glVertex3f(-1.f, 1.f, -1.f);
		}
	}
	glEnd();
	glPopMatrix();
}

static void
circle(float radius, float height, unsigned int accuracy)
{                               /* Draw a disc in current z plane */
	unsigned int i;
	float angle, step;

	step = (2 * (float)F_PI) / accuracy;
	angle = 0;
	glNormal3f(0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.f, 0.f, height);
	for (i = 0; i <= accuracy; i++) {
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);
		angle -= step;
	}
	glEnd();
}

static void
circleSloped(float radius, float startHeight, float endHeight, unsigned int accuracy)
{                               /* Draw a disc in sloping z plane */
	unsigned int i;
	float angle, step;

	step = (2 * (float)F_PI) / accuracy;
	angle = 0;
	glNormal3f(0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.f, 0.f, startHeight);
	for (i = 0; i <= accuracy; i++) {
		float height = ((cosf(angle) + 1) / 2) * (endHeight - startHeight);
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, startHeight + height);
		angle -= step;
	}
	glEnd();
}

static void
circleRev(float radius, float height, unsigned int accuracy)
{                               /* Draw a disc with reverse winding in current z plane */
	unsigned int i;
	float angle, step;

	step = (2 * (float)F_PI) / accuracy;
	angle = 0;
	glNormal3f(0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.f, 0.f, height);
	for (i = 0; i <= accuracy; i++) {
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);
		angle += step;
	}
	glEnd();
}

static void
circleTex(float radius, float height, unsigned int accuracy, const Texture* texture)
{                               /* Draw a disc in current z plane with a texture */
	unsigned int i;
	float angle, step;

	if (!texture) {
		circle(radius, height, accuracy);
		return;
	}

	step = (2 * (float)F_PI) / accuracy;
	angle = 0;
	glNormal3f(0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(.5f, .5f);
	glVertex3f(0.f, 0.f, height);
	for (i = 0; i <= accuracy; i++) {
		glTexCoord2f((sinf(angle) * radius + radius) / (radius * 2), (cosf(angle) * radius + radius) / (radius * 2));
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);
		angle -= step;
	}
	glEnd();
}

static void
circleRevTex(float radius, float height, unsigned int accuracy, const Texture* texture)
{                               /* Draw a disc with reverse winding in current z plane with a texture */
	unsigned int i;
	float angle, step;

	if (!texture) {
		circleRev(radius, height, accuracy);
		return;
	}

	step = (2 * (float)F_PI) / accuracy;
	angle = 0;
	glNormal3f(0.f, 0.f, 1.f);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(.5f, .5f);
	glVertex3f(0.f, 0.f, height);
	for (i = 0; i <= accuracy; i++) {
		glTexCoord2f((sinf(angle) * radius + radius) / (radius * 2), (cosf(angle) * radius + radius) / (radius * 2));
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);
		angle += step;
	}
	glEnd();
}

static void
cylinder(float radius, float height, unsigned int accuracy, const Texture* texture)
{
	unsigned int i;
	float angle = 0;
	float circum = (float)F_PI * radius * 2 / (accuracy + 1);
	float step = (2 * (float)F_PI) / accuracy;
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < accuracy + 1; i++) {
		glNormal3f(sinf(angle), cosf(angle), 0.f);
		if (texture)
			glTexCoord2f(circum * i / (radius * 2), 0.f);
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, 0.f);

		if (texture)
			glTexCoord2f(circum * i / (radius * 2), height / (radius * 2));
		glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);

		angle += step;
	}
	glEnd();
}

#define CACHE_SIZE	240
static void gluCylinderMine(GLfloat baseRadius, GLfloat topRadius, GLfloat height, int slices, int stacks, int textureCoords)
{
	int i, j;
	GLfloat sinCache[CACHE_SIZE];
	GLfloat cosCache[CACHE_SIZE];
	GLfloat sinCache3[CACHE_SIZE];
	GLfloat cosCache3[CACHE_SIZE];
	GLfloat angle;
	GLfloat zLow, zHigh;
	GLfloat length;
	GLfloat deltaRadius;
	GLfloat zNormal;
	GLfloat xyNormalRatio;
	GLfloat radiusLow, radiusHigh;

	if (slices >= CACHE_SIZE) slices = CACHE_SIZE - 1;

	if (slices < 2 || stacks < 1 || baseRadius < 0.0 || topRadius < 0.0 ||
		height < 0.0) {
		return;
	}

	/* Compute length (needed for normal calculations) */
	deltaRadius = baseRadius - topRadius;
	length = sqrtf(deltaRadius * deltaRadius + height * height);
	if (length == 0.0) {
		return;
	}

	/* Cache is the vertex locations cache */
	/* Cache3 is the various normals for the faces */

	zNormal = deltaRadius / length;
	xyNormalRatio = height / length;

	for (i = 0; i < slices; i++) {
		angle = 2 * F_PI * i / slices;
		sinCache[i] = sinf(angle);
		cosCache[i] = cosf(angle);
	}

	for (i = 0; i < slices; i++) {
		angle = 2 * F_PI * (i - 0.5f) / slices;
		sinCache3[i] = xyNormalRatio * sinf(angle);
		cosCache3[i] = xyNormalRatio * cosf(angle);
	}

	sinCache[slices] = sinCache[0];
	cosCache[slices] = cosCache[0];
	sinCache3[slices] = sinCache3[0];
	cosCache3[slices] = cosCache3[0];

	/* Note:
	** An argument could be made for using a TRIANGLE_FAN for the end
	** of the cylinder of either radii is 0.0 (a cone).  However, a
	** TRIANGLE_FAN would not work in smooth shading mode (the common
	** case) because the normal for the apex is different for every
	** triangle (and TRIANGLE_FAN doesn't let me respecify that normal).
	** Now, my choice is GL_TRIANGLES, or leave the GL_QUAD_STRIP and
	** just let the GL trivially reject one of the two triangles of the
	** QUAD.  GL_QUAD_STRIP is probably faster, so I will leave this code
	** alone.
	*/
	for (j = 0; j < stacks; j++) {
		zLow = j * height / stacks;
		zHigh = (j + 1) * height / stacks;
		radiusLow = baseRadius - deltaRadius * ((float)j / stacks);
		radiusHigh = baseRadius - deltaRadius * ((float)(j + 1) / stacks);

		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= slices; i++) {
			glNormal3f(sinCache3[i], cosCache3[i], zNormal);
			if (textureCoords)
				glTexCoord2f(1 - (float)i / slices, (float)j / stacks);

			glVertex3f(radiusLow * sinCache[i],
				radiusLow * cosCache[i], zLow);
			if (textureCoords)
				glTexCoord2f(1 - (float)i / slices, (float)(j + 1) / stacks);

			glVertex3f(radiusHigh * sinCache[i],
				radiusHigh * cosCache[i], zHigh);
		}
		glEnd();
	}
}

static void gluDiskMine(GLfloat innerRadius, GLfloat outerRadius, int slices, int loops, int textureCoords)
{
	GLfloat startAngle = 0.0f;
	GLfloat sweepAngle = 360.0f;

	int  i, j;
	GLfloat sinCache[CACHE_SIZE];
	GLfloat cosCache[CACHE_SIZE];
	GLfloat angle;
	GLfloat deltaRadius;
	GLfloat radiusLow, radiusHigh;
	GLfloat texLow = 0.0, texHigh = 0.0;
	GLfloat angleOffset;
	int  slices2;
	int  finish;

	if (slices >= CACHE_SIZE) slices = CACHE_SIZE - 1;
	if (slices < 2 || loops < 1 || outerRadius <= 0.0 || innerRadius < 0.0 ||
		innerRadius > outerRadius) {
		return;
	}

	if (sweepAngle < -360.0) sweepAngle = 360.0;
	if (sweepAngle > 360.0) sweepAngle = 360.0;
	if (sweepAngle < 0) {
		startAngle += sweepAngle;
		sweepAngle = -sweepAngle;
	}

	if (sweepAngle == 360.0) {
		slices2 = slices;
	}
	else {
		slices2 = slices + 1;
	}

	/* Compute length (needed for normal calculations) */
	deltaRadius = outerRadius - innerRadius;

	/* Cache is the vertex locations cache */

	angleOffset = startAngle / 180.0f * F_PI;
	for (i = 0; i <= slices; i++) {
		angle = angleOffset + ((F_PI * sweepAngle) / 180.0f) * i / slices;
		sinCache[i] = sinf(angle);
		cosCache[i] = cosf(angle);
	}

	if (sweepAngle == 360.0) {
		sinCache[slices] = sinCache[0];
		cosCache[slices] = cosCache[0];
	}

	glNormal3f(0.0, 0.0, 1.0);

	if (innerRadius == 0.0) {
		finish = loops - 1;
		/* Triangle strip for inner polygons */
		glBegin(GL_TRIANGLE_FAN);
		if (textureCoords)
			glTexCoord2f(0.5, 0.5);

		glVertex3f(0.0, 0.0, 0.0);
		radiusLow = outerRadius -
			deltaRadius * ((float)(loops - 1) / loops);
		if (textureCoords)
			texLow = radiusLow / outerRadius / 2;

		for (i = slices; i >= 0; i--) {
			if (textureCoords)
				glTexCoord2f(texLow * sinCache[i] + 0.5f, texLow * cosCache[i] + 0.5f);

			glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);
		}

		glEnd();
	}
	else {
		finish = loops;
	}
	for (j = 0; j < finish; j++) {
		radiusLow = outerRadius - deltaRadius * ((float)j / loops);
		radiusHigh = outerRadius - deltaRadius * ((float)(j + 1) / loops);
		if (textureCoords) {
			texLow = radiusLow / outerRadius / 2;
			texHigh = radiusHigh / outerRadius / 2;
		}

		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= slices; i++) {
			if (textureCoords)
				glTexCoord2f(texLow * sinCache[i] + 0.5f, texLow * cosCache[i] + 0.5f);

			glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);

			if (textureCoords)
				glTexCoord2f(texHigh * sinCache[i] + 0.5f, texHigh * cosCache[i] + 0.5f);

			glVertex3f(radiusHigh * sinCache[i], radiusHigh * cosCache[i], 0.0);
		}
		glEnd();
	}
}

static void
circleOutlineOutward(float radius, float height, unsigned int accuracy)
{                               /* Draw an ouline of a disc in current z plane with outfacing normals */
    unsigned int i;
    float angle, step;

    step = (2 * (float) F_PI) / accuracy;
    angle = 0;
    glNormal3f(0.f, 0.f, 1.f);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i <= accuracy; i++) {
        glNormal3f(sinf(angle), cosf(angle), 0.f);
        glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);
        angle -= step;
    }
    glEnd();
}

static void
circleOutline(float radius, float height, unsigned int accuracy)
{                               /* Draw an ouline of a disc in current z plane */
    unsigned int i;
    float angle, step;

    step = (2 * (float) F_PI) / accuracy;
    angle = 0;
    glNormal3f(0.f, 0.f, 1.f);
    glBegin(GL_LINE_STRIP);
    for (i = 0; i <= accuracy; i++) {
        glVertex3f(sinf(angle) * radius, cosf(angle) * radius, height);
        angle -= step;
    }
    glEnd();
}

static void
drawCube(float size)
{                               /* Draw a simple cube */
    glPushMatrix();
    glScalef(size / 2.0f, size / 2.0f, size / 2.0f);

    glBegin(GL_QUADS);
    /* Front Face */
    glVertex3f(-1.f, -1.f, 1.f);
    glVertex3f(1.f, -1.f, 1.f);
    glVertex3f(1.f, 1.f, 1.f);
    glVertex3f(-1.f, 1.f, 1.f);
    /* Top Face */

    glVertex3f(-1.f, 1.f, -1.f);
    glVertex3f(-1.f, 1.f, 1.f);
    glVertex3f(1.f, 1.f, 1.f);
    glVertex3f(1.f, 1.f, -1.f);
    /* Bottom Face */
    glVertex3f(-1.f, -1.f, -1.f);
    glVertex3f(1.f, -1.f, -1.f);
    glVertex3f(1.f, -1.f, 1.f);
    glVertex3f(-1.f, -1.f, 1.f);
    /* Right face */
    glVertex3f(1.f, -1.f, -1.f);
    glVertex3f(1.f, 1.f, -1.f);
    glVertex3f(1.f, 1.f, 1.f);
    glVertex3f(1.f, -1.f, 1.f);
    /* Left Face */
    glVertex3f(-1.f, -1.f, -1.f);
    glVertex3f(-1.f, -1.f, 1.f);
    glVertex3f(-1.f, 1.f, 1.f);
    glVertex3f(-1.f, 1.f, -1.f);
    glEnd();

    glPopMatrix();
}
