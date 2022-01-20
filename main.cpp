// A simple introductory program; its main window contains a static picture
// of a pong, whose three vertices are red, green and blue.  The program
// illustrates viewing with default viewing parameters only.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <iostream>
#include <functional>
#include <cmath>
#include <chrono>
using pong_time = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<float>>;

const float gridx = 0.7;
const float gridy = 1.0;

const float ball_radius = 0.05;
const float ball_speed = 1.5;

const float paddle_height = ball_radius;
const float paddle_depth = 0.2 * ball_radius;
const float paddle_width = 0.15;

float paddle_speed = 1;

const float vision_change = 0.1;

void update(int i);

struct Pong
{
    bool left;
    int leftx, lefty;

    float distance{2};
    
    float longitude{M_PI/4};
    float latitude{M_PI/2};

    std::chrono::time_point<std::chrono::steady_clock> now()
    {
        return std::chrono::steady_clock::now();
    }

    struct Move
    {
        float position_x_, position_y_, position_z_;
        float velocity_x_, velocity_y_, velocity_z_;
        pong_time time_;
        void set(float position_x, float position_y, float position_z,
                 float velocity_x, float velocity_y, float velocity_z,
                 pong_time time)
              {
                position_x_ = position_x;
                position_y_ = position_y;
                position_z_ = position_z;

                velocity_x_ = velocity_x;
                velocity_y_ = velocity_y;
                velocity_z_ = velocity_z;

                time_ = time;
              };
        std::tuple<float,float,float> get_position(pong_time time)
        {
            auto elapsed = time-time_;
            return
            {
                position_x_+velocity_x_*elapsed.count(),
                position_y_+velocity_y_*elapsed.count(),
                position_z_+velocity_z_*elapsed.count()
            };
        }
    };


    struct Paddle_Move: public Move
    {
        std::tuple<float,float,float> get_position(pong_time time)
        {
            auto elapsed = time - time_;
            auto [px,py,pz] = Move::get_position(time);

            if(px-paddle_width < -gridx)
            {
                px = -gridx + paddle_width;
            }

            if(px+paddle_width > gridx)
            {
                px = gridx - paddle_width;
            }

            return {px, py, pz};
        }
    };

    Move ball_move_;
    Paddle_Move paddle_movers_[2];

    void updatex(int i)
    {
        glutTimerFunc(25,update,0);//Call update after each 25 millisecond
        glutPostRedisplay();
    }

    void draw_ball()
    {
        glPushMatrix();
        glColor3f(1,1,1);
        auto [x,y,z] = ball_move_.get_position(now());
        glTranslatef(x, y, z);
        glutSolidSphere(ball_radius, 30, 30);
        glPopMatrix();
    }

    void draw_rectangular_prism(float size_x, float size_y, float size_z)
    {
        glBegin(GL_QUADS);//xy+
            glVertex3f(size_x, size_y, size_z);
            glVertex3f(-size_x, size_y, size_z);
            glVertex3f(-size_x, -size_y, size_z);
            glVertex3f(size_x, -size_y, size_z);
        glEnd();

        glBegin(GL_QUADS);//yz+
            glVertex3f(size_x, size_y, size_z);
            glVertex3f(size_x, size_y, -size_z);
            glVertex3f(size_x, -size_y, -size_z);
            glVertex3f(size_x, -size_y, size_z);
        glEnd();

        glBegin(GL_QUADS);//xz+
            glVertex3f(size_x, size_y, size_z);
            glVertex3f(-size_x, size_y, size_z);
            glVertex3f(-size_x, size_y, -size_z);
            glVertex3f(size_x, size_y, -size_z);
        glEnd();

        glBegin(GL_QUADS);//xy-
            glVertex3f(-size_x, -size_y, -size_z);
            glVertex3f(size_x, -size_y, -size_z);
            glVertex3f(size_x, size_y, -size_z);
            glVertex3f(-size_x, size_y, -size_z);
        glEnd();

        glBegin(GL_QUADS);//xz-
            glVertex3f(-size_x, -size_y, -size_z);
            glVertex3f(size_x, -size_y, -size_z);
            glVertex3f(size_x, -size_y, size_z);
            glVertex3f(-size_x, -size_y, size_z);
        glEnd();

        glBegin(GL_QUADS);//yz-
            glVertex3f(-size_x, -size_y, -size_z);
            glVertex3f(-size_x, size_y, -size_z);
            glVertex3f(-size_x, size_y, size_z);
            glVertex3f(-size_x, -size_y, size_z);
        glEnd();
    }

    void draw_paddle(Paddle_Move &m)
    {
        glPushMatrix();
        auto [x,y,z] = m.get_position(now());
        z < 0 ? glColor3f(1, 0, 0) : glColor3f(0,1,0);
        glTranslatef(x, y, z);
        draw_rectangular_prism(paddle_width, paddle_height, paddle_depth);
        glPopMatrix();
    }
    
    void draw_board()
    {
        glBegin(GL_QUADS);
            glColor3f(0,0,1);
            glVertex3f(gridx, 0, gridy);
            glVertex3f(gridx, 0,-gridy);
            glVertex3f(-gridx, 0,-gridy);
            glVertex3f(-gridx,0, gridy);
        glEnd();
    }

    void check_collision(bool mine)
    {
        if(mine)
        {
            auto[x, y, z] = ball_move_.get_position(now());
            float distance_past_paddle_surface = z - (gridy - ball_radius*0.7 - 2 * paddle_depth);
            if (distance_past_paddle_surface > 0)
            {
                if (ball_move_.velocity_z_ == 0) return;
                auto t = now() - std::chrono::duration<float>(distance_past_paddle_surface / ball_move_.velocity_z_);
                auto[px, py, pz] = paddle_movers_[0].get_position(t);
                auto[bx, by, bz] = ball_move_.get_position(t);

                if (bx < px + paddle_width && bx > px - paddle_width)
                {
                    ball_move_.set(bx, by, bz, ball_move_.velocity_x_, ball_move_.velocity_y_, -ball_move_.velocity_z_, t);
                } else {
                    ball_move_.set(bx, by, bz, 0,0,0, t);
                }
            }
        } else {
            auto[x, y, z] = ball_move_.get_position(now());
            float distance_past_paddle_surface = (-gridy + ball_radius * 0.7 + 2 * paddle_depth) - z;
            if (distance_past_paddle_surface > 0)
            {
                if (ball_move_.velocity_z_ == 0) return;
                auto t = now() - std::chrono::duration<float>(-distance_past_paddle_surface / ball_move_.velocity_z_);
                auto[px, py, pz] = paddle_movers_[1].get_position(t);
                auto[bx, by, bz] = ball_move_.get_position(t);

                if (bx < px + paddle_width && bx > px - paddle_width)
                {
                    ball_move_.set(bx, by, bz, ball_move_.velocity_x_, ball_move_.velocity_y_, -ball_move_.velocity_z_, t);
                } else {
                    ball_move_.set(bx, by, bz, 0,0,0, t);
                }
            }
        }
    }

    void check_sides()
    {
        auto[x,y,z] = ball_move_.get_position(now());

        if(x+ball_radius > gridx)
        {
            auto t = now() - std::chrono::duration<float>(x -(gridx - ball_radius))/ ball_move_.velocity_x_;
            auto[bx,by,bz] = ball_move_.get_position(now());
            ball_move_.set(bx, by, bz, -ball_move_.velocity_x_, ball_move_.velocity_y_, ball_move_.velocity_z_, t);
            return;
        }
        if(x-ball_radius < -gridx)
        {
            auto t = now() - std::chrono::duration<float>( (x + gridx - ball_radius))/ ball_move_.velocity_x_;
            auto[bx,by,bz] = ball_move_.get_position(now());
            ball_move_.set(bx, by, bz, -ball_move_.velocity_x_, ball_move_.velocity_y_, ball_move_.velocity_z_, t);;
        }

    }

    void display()
    {
        check_collision(true);
        check_collision(false);
        check_sides();

        if(longitude < 0.1) longitude = 0.1;
        if(longitude >= M_PI/2-0.001) longitude = M_PI/2-0.001;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60, static_cast<float>(glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT),  0.01, 100);
        float eye_x = distance*cosf(longitude) * cosf(latitude);
        float eye_z = distance*cosf(longitude) * sinf(latitude);
        float eye_y = distance*sinf(longitude);
        gluLookAt(eye_x, eye_y, eye_z, 0, 0, 0, 0, 1, 0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        //glutSolidTeapot(1);
        draw_board();
        draw_ball();


        for(auto &i : paddle_movers_)
        {
            draw_paddle(i);
        }

        glutSwapBuffers();
        glutTimerFunc(25,update,0);//Call update after each 25 millisecond
    }


    void mouse(int button, int state, int x, int y)
    {
        if(button == GLUT_LEFT_BUTTON)
        {
            left = (state == GLUT_DOWN);
            leftx = x;
            lefty = y;
        }
    }

    void keys(unsigned char c, int x, int y)
    {
        //std::cout << "got key '" << c << "', " << (unsigned int)(c) << '\n';
        if(c == 'q')
        {
            exit(0);
            // glutLeaveGameMode();
        }
        if(c == 'r')
        {
            start();
        }
        if(c == '+')
        {
            distance /=1.1;
        }
        if(c== '-')
        {
            distance *= 1.1;
        }
        if(c == 'a')
        {
            auto [x,y,z] = paddle_movers_[1].get_position(now());
            paddle_movers_[1].set(x, y, z, -paddle_speed, 0, 0, now());
        }
        if(c == 'd')
        {
            auto [x,y,z] = paddle_movers_[1].get_position(now());
            paddle_movers_[1].set(x, y, z, paddle_speed, 0, 0, now());
        }

        glutPostRedisplay();
    }

    void keysup(unsigned char c, int x, int y)
    {
        if(c == 'a' || c == 'd')
        {
            auto [x,y,z] = paddle_movers_[1].get_position(now());
            paddle_movers_[1].set(x, y, z, 0, 0, 0, now());
        }
    }

    void special(int c, int x, int y)
    {
        if(c == GLUT_KEY_LEFT)
        {
            //std::cout << "LEFT\n";
            auto [x,y,z] = paddle_movers_[0].get_position(now());
            paddle_movers_[0].set(x, y, z, -paddle_speed, 0, 0, now());
        }
        if(c == GLUT_KEY_DOWN)
        {
            start();
        }
        if(c == GLUT_KEY_RIGHT)
        {
            //std::cout << "LEFT\n";
            auto [x,y,z] = paddle_movers_[0].get_position(now());
            paddle_movers_[0].set(x, y, z, paddle_speed, 0, 0, now());
        }

    }

    void specialup(int c, int x, int y)
    {
        if(c == GLUT_KEY_LEFT || GLUT_KEY_RIGHT)
        {
            auto [x,y,z] = paddle_movers_[0].get_position(now());
            paddle_movers_[0].set(x, y, z, 0, 0, 0, now());
        }
    }

    void motion(int x, int y)
    {
        if(left)
        {
            latitude += static_cast<float>(x - leftx) / glutGet(GLUT_WINDOW_WIDTH)*4;
            leftx = x;
            longitude += static_cast<float>(y - lefty) / glutGet(GLUT_WINDOW_WIDTH)*4;
            lefty = y;

            glutPostRedisplay();
        }
    }

    void start()
    {
        pong_time t{now()};
        ball_move_.set(0,ball_radius,0,0.4*ball_speed,0,0.5*ball_speed,t);
        paddle_movers_[0].set(0, paddle_height, gridy - paddle_depth, 0, 0, 0, now());
        paddle_movers_[1].set(0, paddle_height, -gridy + paddle_depth, 0, 0, 0, now());

        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);

        glutMainLoop();
    }
};

Pong pong;
void display() { pong.display(); }
void update(int i){pong.updatex(i);}
void mouse(int button, int state, int x, int y) { pong.mouse(button, state, x, y); }
void keys(unsigned char c, int x, int y) { pong.keys(c, x, y); }
void keysup(unsigned char c, int x, int y) { pong.keysup(c, x, y); }
void motion(int x, int y) { pong.motion(x, y);}
void special(int c, int x, int y) { pong.special(c, x, y); }
void specialup(int c, int x, int y) { pong.specialup(c, x, y); }


int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("test");

//
//    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//    GLfloat mat_shininess[] = { 100.0 };
//    GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
//    glClearColor (0.0, 0.0, 0.0, 0.0);
//    glShadeModel (GL_SMOOTH);
//
//    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
//    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
//    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
// +
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
//    glEnable(GL_DEPTH_TEST);
    // famous GLUT callback functions

    glutDisplayFunc(display);
    glutKeyboardFunc(keys);
    glutKeyboardUpFunc(keysup);

    glutSpecialFunc(special);
    glutSpecialUpFunc(specialup);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    pong.start();

    return 0;
}