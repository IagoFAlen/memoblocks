#pragma once

namespace handle {
    enum Status {
        NULLABLE   = -2,
        SIZE_ERROR = -1,
        FAILURE    = 0,
        SUCCESS    = 1,
    };
}