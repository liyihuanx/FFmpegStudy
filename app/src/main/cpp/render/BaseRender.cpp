//
// Created by leeyh on 2022/9/27.
//

#include "BaseRender.h"

BaseRender::BaseRender(int type) {
    render_type = type;
}

BaseRender::~BaseRender() = default;

int BaseRender::getRenderType() {
    return render_type;
}
