#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include "tracer.hpp"

namespace PathTracer {
    class Controller : public PTObject {
        bool monteCarlo = true;

    public:
        glm::vec3 eye = glm::vec3(6.0f, 8.0f, 0.0f);
        glm::vec3 view = glm::vec3(0.0f, 6.0f, 0.0f);
        glm::vec2 mposition;
        Tracer * raysp;

        glm::mat4 project() {
            return glm::lookAt(eye, view, glm::vec3(0.0f, 1.0f, 0.0f));
        }

        void setRays(Tracer * r) {
            raysp = r;
        }

        void work(glm::vec2 &position, pgl::doublev &diff, bool &mouseleft, bool keys[10]) {
            glm::mat4 viewm = project();
            glm::mat4 unviewm = glm::inverse(viewm);
            glm::vec3 ca = (viewm * glm::vec4(eye, 1.0f)).xyz();
            glm::vec3 vi = (viewm * glm::vec4(view, 1.0f)).xyz();

            bool isFocus = true;

            if (mouseleft && isFocus)
            {
                glm::vec2 mpos = glm::vec2(position) - mposition;
                pgl::floatv diffX = mpos.x;
                pgl::floatv diffY = mpos.y;
                this->rotateX(vi, diffX);
                this->rotateY(vi, diffY);
                if (monteCarlo) raysp->clearSampler();
            }
            mposition = glm::vec2(position);

            if (keys[kW] && isFocus)
            {
                forwardBackward(ca, vi, diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if (keys[kS] && isFocus)
            {
                forwardBackward(ca, vi, -diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if (keys[kA] && isFocus)
            {
                leftRight(ca, vi, diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if (keys[kD] && isFocus)
            {
                leftRight(ca, vi, -diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if ((keys[kE] || keys[kSpc]) && isFocus)
            {
                topBottom(ca, vi, diff);
                if (monteCarlo) raysp->clearSampler();
            }

            if ((keys[kQ] || keys[kSft] || keys[kC]) && isFocus)
            {
                topBottom(ca, vi, -diff);
                if (monteCarlo) raysp->clearSampler();
            }

            eye  = (unviewm * glm::vec4(ca, 1.0f)).xyz();
            view = (unviewm * glm::vec4(vi, 1.0f)).xyz();
        }

        void leftRight(glm::vec3 &ca, glm::vec3 &vi, pgl::floatv diff) {
            ca.x -= diff / 100.0f;
            vi.x -= diff / 100.0f;
        }
        void topBottom(glm::vec3 &ca, glm::vec3 &vi, pgl::floatv diff) {
            ca.y += diff / 100.0f;
            vi.y += diff / 100.0f;
        }
        void forwardBackward(glm::vec3 &ca, glm::vec3 &vi, pgl::floatv diff) {
            ca.z -= diff / 100.0f;
            vi.z -= diff / 100.0f;
        }
        void rotateY(glm::vec3 &vi, pgl::floatv diff) {
            glm::mat4 rot;
            rot = glm::rotate(rot, (-diff / pgl::floatv(raysp->displayHeight) / 0.5f), glm::vec3(1.0f, 0.0f, 0.0f));
            vi = (rot * glm::vec4(vi, 1.0f)).xyz();
        }
        void rotateX(glm::vec3 &vi, pgl::floatv diff) {
            glm::mat4 rot;
            rot = glm::rotate(rot, (-diff / pgl::floatv(raysp->displayHeight) / 0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
            vi = (rot * glm::vec4(vi, 1.0f)).xyz();
        }
    };
}
