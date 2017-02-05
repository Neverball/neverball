#pragma once

#include "includes.hpp"
#include "utils.hpp"
#include "tracer.hpp"

namespace PathTracer {
    class Controller : public PTObject {
        pgl::boolv monteCarlo = true;

    public:
        pgl::floatv3 eye = pgl::floatv3(6.0f, 8.0f, 0.0f);
        pgl::floatv3 view = pgl::floatv3(0.0f, 6.0f, 0.0f);
        pgl::floatv2 mposition;
        Tracer * raysp;

        glm::mat4 project() {
            return glm::lookAt(eye, view, pgl::floatv3(0.0f, 1.0f, 0.0f));
        }

        void setRays(Tracer * r) {
            raysp = r;
        }

        void work(pgl::floatv2 &position, pgl::doublev &diff, pgl::boolv &mouseleft, pgl::boolv keys[10]) {
            glm::mat4 viewm = project();
            glm::mat4 unviewm = glm::inverse(viewm);
            pgl::floatv3 ca = (viewm * pgl::floatv4(eye, 1.0f)).xyz();
            pgl::floatv3 vi = (viewm * pgl::floatv4(view, 1.0f)).xyz();

            pgl::boolv isFocus = true;

            if (mouseleft && isFocus)
            {
                pgl::floatv2 mpos = pgl::floatv2(position) - mposition;
                pgl::floatv diffX = mpos.x;
                pgl::floatv diffY = mpos.y;
                this->rotateX(vi, diffX);
                this->rotateY(vi, diffY);
                if (monteCarlo) raysp->clearSampler();
            }
            mposition = pgl::floatv2(position);

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

            eye  = (unviewm * pgl::floatv4(ca, 1.0f)).xyz();
            view = (unviewm * pgl::floatv4(vi, 1.0f)).xyz();
        }

        void leftRight(pgl::floatv3 &ca, pgl::floatv3 &vi, pgl::floatv diff) {
            ca.x -= diff / 100.0f;
            vi.x -= diff / 100.0f;
        }
        void topBottom(pgl::floatv3 &ca, pgl::floatv3 &vi, pgl::floatv diff) {
            ca.y += diff / 100.0f;
            vi.y += diff / 100.0f;
        }
        void forwardBackward(pgl::floatv3 &ca, pgl::floatv3 &vi, pgl::floatv diff) {
            ca.z -= diff / 100.0f;
            vi.z -= diff / 100.0f;
        }
        void rotateY(pgl::floatv3 &vi, pgl::floatv diff) {
            glm::mat4 rot;
            rot = glm::rotate(rot, (-diff / pgl::floatv(raysp->displayHeight) / 0.5f), pgl::floatv3(1.0f, 0.0f, 0.0f));
            vi = (rot * pgl::floatv4(vi, 1.0f)).xyz();
        }
        void rotateX(pgl::floatv3 &vi, pgl::floatv diff) {
            glm::mat4 rot;
            rot = glm::rotate(rot, (-diff / pgl::floatv(raysp->displayHeight) / 0.5f), pgl::floatv3(0.0f, 1.0f, 0.0f));
            vi = (rot * pgl::floatv4(vi, 1.0f)).xyz();
        }
    };
}
