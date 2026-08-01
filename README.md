RasterFont for SDL2

This is a project mainly for my own education and gratification. I don't really like SDL-TTF. When I tried it it looked bad to me, especially when scaled. I probably just wasn't diving deep enough into the settings for aliasing or interpolation or whatnot, but I kind of don't want to have to.
In previous projects I've used a wonderful drop-in header library to turn TTFs to bitmap font, but for a current project I wanted something scalable and usable on my own terms. I'm building this first font in actual Notepad from 16x16 letters I'm hacking out in YY-CHR. An artist/typographer I am not. But it's kind of Missile Commandy, which is what I wanted.
If this little font library can help you in your SDL projects, you're welcome to it. I have a lot of work to do on it. My TODO list is right at the top of rasterfont.h. I will also be extending this to SDL3 at the same time because I'm building the project with both libraries, but that will be a different repo.
