var gulp = require("gulp");
var gzip = require("gulp-gzip");
var del = require("del");

gulp.task("compress", ["clean"], function() {
  gulp
    .src(["./dist/**/*.css","./dist/**/*.js", "./dist/**/*.html", "./dist/**/*.ico"])
    .pipe(gzip())
    .pipe(gulp.dest("../data"));
});
gulp.task("clean", function() {
  return del("../data/**", { force: true });
});
gulp.task("default", ["compress"]);

// -----------------------------------------------------------------------------
// PlatformIO support
// -----------------------------------------------------------------------------
 
// const spawn = require('child_process').spawn;
// const argv = require('yargs').argv;
 
// var platformio = function(target) {
//     var args = ['run'];
//     if ("e" in argv) { args.push('-e'); args.push(argv.e); }
//     if ("p" in argv) { args.push('--upload-port'); args.push(argv.p); }
//     if (target) { args.push('-t'); args.push(target); }
//     const cmd = spawn('platformio', args);
//     cmd.stdout.on('data', function(data) { console.log(data.toString().trim()); });
//     cmd.stderr.on('data', function(data) { console.log(data.toString().trim()); });
// }
 
// gulp.task('uploadfs', function() { platformio('uploadfs'); });
// gulp.task('upload', function() { platformio('upload'); });
// gulp.task('run', function() { platformio(false); });
