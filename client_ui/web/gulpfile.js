const { series, src, dest } = require('gulp')
const path = require('path')

function copyCommonWeb(cb) {
    return src(path.join(__dirname, '..', 'common_web', 'dist', 'dev', 'web', '**', '*'))
        .pipe(dest('dist'))
}

function copyAssets() {
    return src(path.join(__dirname, '..', 'assets', '**', '*'))
        .pipe(dest('assets'))
}
exports.default = series(copyCommonWeb, copyAssets)