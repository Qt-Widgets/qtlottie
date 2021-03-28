#include "SkottieWrapper.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "src/utils/SkOSPath.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/skottie/include/Skottie.h"

struct Skottie_Animation_Private
{
    sk_sp<skresources::CachingResourceProvider> rp;
    sk_sp<skottie::Animation> ani;
    sk_sp<SkSurface> surface;

};


Skottie_Animation* skottie_animation_from_file(const char* path, const char* resource)
{
    Skottie_Animation_Private *ani = new Skottie_Animation_Private{};
    if(!ani)
        return nullptr;

    ani->rp = skresources::CachingResourceProvider::Make(
            skresources::DataURIResourceProviderProxy::Make(
                skresources::FileResourceProvider::Make(SkOSPath::Dirname(resource),
                    /*predecode=*/true), /*predecode=*/true));

    ani->ani = skottie::Animation::Builder()
            .setResourceProvider(ani->rp)
            .makeFromFile(path);

    SkSize aniSize = ani->ani->size();
    ani->surface = SkSurface::MakeRasterN32Premul(aniSize.width(), aniSize.height());

    return reinterpret_cast<Skottie_Animation*>(ani);
}

Skottie_Animation* skottie_animation_fromdata(void *data, size_t data_size, const char *resource)
{
    Skottie_Animation_Private *ani = new Skottie_Animation_Private{};
    if(!ani)
        return nullptr;

    ani->rp = skresources::CachingResourceProvider::Make(
            skresources::DataURIResourceProviderProxy::Make(
                skresources::FileResourceProvider::Make(SkOSPath::Dirname(resource),
                    /*predecode=*/true), /*predecode=*/true));

    ani->ani = skottie::Animation::Builder()
            .setResourceProvider(ani->rp)
            .make(reinterpret_cast<const char*>(data), data_size);

    SkSize aniSize = ani->ani->size();
    ani->surface = SkSurface::MakeRasterN32Premul(aniSize.width(), aniSize.height());

    return reinterpret_cast<Skottie_Animation*>(ani);
}

Skottie_Pixmap* skottie_new_pixmap()
{
    return  reinterpret_cast<Skottie_Pixmap*>(new SkPixmap{});
}

const void* skottie_get_pixmap_buffer(Skottie_Pixmap* pixmap)
{
    SkPixmap *skPixmap = reinterpret_cast<SkPixmap*>(pixmap);
    return reinterpret_cast<const void*>(skPixmap->addr());
}

void skottie_delete_pixmap(Skottie_Pixmap* pixmap)
{
    delete reinterpret_cast<SkPixmap*>(pixmap);
}

void skottie_animation_render(Skottie_Animation *animation,
                                         size_t frame_num,
                                 Skottie_Pixmap *pixmap)
{
    if(!animation)
        return;
    Skottie_Animation_Private *pAni =
            reinterpret_cast<Skottie_Animation_Private*>(animation);
    auto canvas = pAni->surface->getCanvas();
    if(!canvas)
        return;
    canvas->clear(SkColorSetARGB(0,0,0,0));

    pAni->ani->seekFrame(static_cast<double>(frame_num));
    pAni->ani->render(canvas);

    auto img = pAni->surface->makeImageSnapshot();
    img->peekPixels(reinterpret_cast<SkPixmap*>(pixmap));
    return;
}

void skottie_animation_destroy(Skottie_Animation *animation)
{
    delete reinterpret_cast<Skottie_Animation_Private*>(animation);
}
