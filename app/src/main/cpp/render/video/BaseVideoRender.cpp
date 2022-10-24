//
// Created by leeyh on 2022/9/27.
//

#include "BaseVideoRender.h"

BaseVideoRender::BaseVideoRender(int type) {
    render_type = type;
}

BaseVideoRender::~BaseVideoRender() = default;

int BaseVideoRender::getRenderType() {
    return render_type;
}
