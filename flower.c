#include <GL/glut.h>
#include <math.h>

float angle       = 0.0f;
float bloomFactor = 0.0f;
float bloomDir    = 0.003f;
int   complexity  = 24;

#define COLLAPSE_SCALE 2.36603f

// How far past the center the corner vertices travel (in sphere radii).
// 0.0 = stop at center, 0.5 = tips poke halfway out the other side
#define OVERSHOOT 0.6f

void drawFace(float ox, float oy, float oz,
              float vx_x, float vx_y, float vx_z,
              float vy_x, float vy_y, float vy_z)
{
    float stageA, stageB;
    if (bloomFactor <= 0.5f) {
        stageA = bloomFactor * 2.0f;
        stageB = 0.0f;
    } else {
        stageA = 1.0f;
        stageB = (bloomFactor - 0.5f) * 2.0f;
    }

    glBegin(GL_QUADS);
    for (int i = 0; i < complexity; i++) {
        for (int j = 0; j < complexity; j++) {

            float ts[4], us[4];
            ts[0] = (float)i       / complexity * 2.0f - 1.0f;
            us[0] = (float)j       / complexity * 2.0f - 1.0f;
            ts[1] = (float)(i + 1) / complexity * 2.0f - 1.0f;
            us[1] = (float)j       / complexity * 2.0f - 1.0f;
            ts[2] = (float)(i + 1) / complexity * 2.0f - 1.0f;
            us[2] = (float)(j + 1) / complexity * 2.0f - 1.0f;
            ts[3] = (float)i       / complexity * 2.0f - 1.0f;
            us[3] = (float)(j + 1) / complexity * 2.0f - 1.0f;

            for (int k = 0; k < 4; k++) {
                float t = ts[k];
                float u = us[k];

                // --- Cube vertex ---
                float cx = ox + t * vx_x + u * vy_x;
                float cy = oy + t * vx_y + u * vy_y;
                float cz = oz + t * vx_z + u * vy_z;

                // --- Sphere vertex (normalized to unit sphere) ---
                float len = sqrtf(cx*cx + cy*cy + cz*cz);
                float sx = cx / len;
                float sy = cy / len;
                float sz = cz / len;

                // Stage A: cube -> sphere
                float ax = cx + stageA * (sx - cx);
                float ay = cy + stageA * (sy - cy);
                float az = cz + stageA * (sz - cz);

                // Stage B: collapse toward center (0,0,0) but PIN the lobe tip.
                //
                // How much a vertex collapses depends on how far it sits from the
                // face normal axis. We measure this as the dot product of the
                // sphere vertex with the face normal (ox,oy,oz).
                // - Face center vertex: dot = 1.0  → collapseWeight = 0  → stays pinned
                // - 45-degree edge:     dot ≈ 0.58 → collapseWeight ≈ 0.42
                // - Corner vertex:      dot ≈ 0.58 → same, all corners share same dot
                //
                // collapseWeight = 1 - dot(sphereVertex, faceNormal)
                // This is zero at the tip and increases toward the sides/corners,
                // so those vertices travel inward to the center while the tip stays.
                //
                // The sides stay CONNECTED because adjacent faces share the same
                // sphere vertices at their shared edges, and those vertices collapse
                // to the same point (0,0,0) regardless of which face computes them.

                float dot = sx * ox + sy * oy + sz * oz;
                float collapseWeight = (1.0f - dot) * COLLAPSE_SCALE;  // 0 at tip, ~0.42 at corners

                float collapse = stageB * collapseWeight;

                float px = ax * (1.0f - collapse);
                float py = ay * (1.0f - collapse);
                float pz = az * (1.0f - collapse);

                glVertex3f(px, py, pz);
            }
        }
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(angle, 1.0f, 0.7f, 0.3f);

    glColor3f(1.0f, 0.2f, 0.2f); drawFace( 0,  0,  1,  1, 0, 0,  0, 1, 0);
    glColor3f(0.2f, 1.0f, 0.2f); drawFace( 0,  0, -1, -1, 0, 0,  0, 1, 0);
    glColor3f(0.3f, 0.5f, 1.0f); drawFace( 1,  0,  0,  0, 0,-1,  0, 1, 0);
    glColor3f(1.0f, 1.0f, 0.1f); drawFace(-1,  0,  0,  0, 0, 1,  0, 1, 0);
    glColor3f(1.0f, 0.2f, 1.0f); drawFace( 0,  1,  0,  1, 0, 0,  0, 0,-1);
    glColor3f(0.1f, 1.0f, 1.0f); drawFace( 0, -1,  0,  1, 0, 0,  0, 0, 1);

    glutSwapBuffers();
}

void idle() {
    angle += 0.8f;
    bloomFactor += bloomDir;
    if (bloomFactor >= 1.5f)
      {
          bloomFactor = 1.5f;
          bloomDir = -bloomDir;
      }
    if (bloomFactor <= -1.0f)
      {
          bloomFactor = -1.0f;
          bloomDir = -bloomDir;
      }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Windows 3D FlowerBox - Natural Lobes");
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
