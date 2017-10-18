#include <jni.h>
#include <android/log.h>

#define LOG_I(...) __android_log_print(ANDROID_LOG_ERROR , "main", __VA_ARGS__)

#include "SDL.h"
#include "SDL_log.h"
#include "SDL_main.h"

////avcodec:编解码(最重要的库)
//#include "libavcodec/avcodec.h"
////avformat:封装格式处理
//#include "libavformat/avformat.h"
////avutil:工具库(大部分库都需要这个库的支持)
//#include "libavutil/imgutils.h"
////swscale:视频像素数据格式转换
//#include "libswscale/swscale.h"
////导入音频采样数据格式转换库
//#include "libswresample/swresample.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;


/* Audio Callback
 * @param stream: A pointer to the audio buffer to be filled
 * @param len: The length (in bytes) of the audio buffer
*/
void  fill_audio(void *udata,Uint8 *stream,int len){
    //SDL 2.0,SDL2和SDL1.x关于视频方面的API差别很大。但是SDL2和SDL1.x关于音频方面的API是一模一样的
    //唯独在回调函数中，SDL2有一个地方和SDL1.x不一样：SDL2中必须首先使用SDL_memset()将stream中的数据设置为0
//    注意：mp3为什么播放不顺畅？
//    len=4096;audio_len=4608;两个相差512！为了这512，还得再调用一次回调函数。。。
//    m4a,aac就不存在此问题(都是4096)！
    SDL_memset(stream, 0, len);
    if(audio_len==0)
        return;
    len=(len>audio_len?audio_len:len);
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

//int main(int argc, char *argv[]) {
//    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
//        LOG_I("SDL_Init failed %s", SDL_GetError());
//        return 0;
//    }
//
//    //编写你的代码
//
//    LOG_I("SDL_Init Success!");
//
//    SDL_Quit();
//    return 0;
//}


//int main(int argc, char *argv[]) {
//    //第一步：初始化SDL多媒体框架->SDL_Init
//    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
//        LOG_I("SDL_Init failed %s", SDL_GetError());
//        return 0;
//    }
//    LOG_I("SDL_Init Success!");
//
//    //第二步：初始化SDL窗口
//    //参数一：窗口名称->要求必需是UTF-8编码
//    //参数二：窗口在屏幕上面X坐标
//    //参数三：窗口在屏幕上面Y坐标
//    //参数四：窗口在屏幕上面宽
//    int width = 640;
//    //参数五：窗口在屏幕上面高
//    int height = 352;
//    //参数六：窗口状态(打开的状态:SDL_WINDOW_OPENGL)
//    SDL_Window* sdl_window = SDL_CreateWindow("Dream开车",
//                                              SDL_WINDOWPOS_CENTERED,
//                                              SDL_WINDOWPOS_CENTERED,
//                                              width ,
//                                              height,
//                                              SDL_WINDOW_OPENGL);
//    if (sdl_window == NULL){
//        LOG_I("窗口创建失败");
//        return 0;
//    }
//
//    //第三步：创建渲染器->渲染窗口(OpenGL ES)
//    //最新一期VIP课程
//    //参数一：渲染目标窗口
//    //参数二：从哪里开始渲染(-1:默认从第一个为止开始)
//    //参数三：渲染类型
//    //SDL_RENDERER_SOFTWARE:软件渲染
//    //...
//    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
//
//    //第四步：创建纹理
//    //参数一：纹理目标渲染器
//    //参数二：渲染格式
//    //参数三：绘制方式(SDL_TEXTUREACCESS_STREAMING:频繁绘制)
//    //参数四：纹理宽
//    //参数五：纹理高
//    SDL_Texture * sdl_texture = SDL_CreateTexture(sdl_renderer,
//                                                  SDL_PIXELFORMAT_IYUV,
//                                                  SDL_TEXTUREACCESS_STREAMING,
//                                                  width,
//                                                  height);
//
//    //第五步：设置纹理数据->播放YUV视频
//    //着色器语言(着色器)、渲染器、纹理等等...
//    //第一点：打开YUV文件(手机：)
//    FILE* yuv_file = fopen("/storage/emulated/0/DreamTestFile/Test.yuv","rb+");
//    if (yuv_file == NULL){
//        LOG_I("文件打开失败");
//        return 0;
//    }
//
//    //第二点：循环读取YUV视频像素数据格式每一帧画面->渲染->设置纹理数据
//    //定义缓冲区(内存空间开辟多大?)
//    //Y:U:V = 4 : 1 : 1
//    //假设：Y = 1.0  U = 0.25  V = 0.25
//    //宽度：Y + U + V = 1.5
//    //换算：Y + U + V = width * height * 1.5
//    char buffer_pix[width * height * 3 / 2];
//
//    //定义渲染器区域
//    SDL_Rect sdl_rect;
//    while (true){
//        //一行一行的读取
//        fread(buffer_pix, 1, width * height * 3 / 2, yuv_file);
//        //判定是否读取完毕
//        if (feof(yuv_file)){
//            break;
//        }
//
//        //设置纹理数据
//        //参数一：目标纹理对象
//        //参数二：渲染区域(NULL:表示默认屏幕窗口宽高)
//        //参数三：视频像素数据
//        //参数四：帧画面宽
//        SDL_UpdateTexture(sdl_texture, NULL, buffer_pix, width);
//
//        //第六步：将纹理数据拷贝到渲染器
//        sdl_rect.x = 0;
//        sdl_rect.y = 0;
//        sdl_rect.w = width;
//        sdl_rect.h = height;
//
//        //先清空
//        SDL_RenderClear(sdl_renderer);
//        //再渲染
//        SDL_RenderCopy(sdl_renderer,sdl_texture,NULL,&sdl_rect);
//
//        //第七步：显示帧画面
//        SDL_RenderPresent(sdl_renderer);
//
//        //第八步：延时渲染(没渲染一帧间隔时间)
//        SDL_Delay(20);
//    }
//
//
//    //第九步：是否内存
//    fclose(yuv_file);
//
//    SDL_DestroyTexture(sdl_texture);
//
//    SDL_DestroyRenderer(sdl_renderer);
//
//
//    //第十步：推出SDL程序
//    SDL_Quit();
//    return 0;
//}

//int main(int argc, char *argv[]) {
//    const char *cinputFilePath = "/storage/emulated/0/Test.mov";
//    //第一步：注册所有组件
//    av_register_all();
//    //支持网络流输入
//    avformat_network_init();
//    //第二步：打开视频输入文件
//    //参数一：封装格式上下文->AVFormatContext->包含了视频信息(视频格式、大小等等...)
//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//    //参数二：打开文件(入口文件)->url
//    int avformat_open_result = avformat_open_input(&pFormatCtx, cinputFilePath, NULL, NULL);
//    if (avformat_open_result != 0) {
//        //获取异常信息
//        char *error_info;
//        av_strerror(avformat_open_result, error_info, 1024);
//        __android_log_print(ANDROID_LOG_INFO, "main", "异常信息：%s", error_info);
//        return 0;
//    }
//    //第三步：查找视频文件信息
//    //参数一：封装格式上下文->AVFormatContext
//    //参数二：配置
//    //返回值：0>=返回OK，否则失败
//    int avformat_find_stream_info_result = avformat_find_stream_info(pFormatCtx, NULL);
//    if (avformat_find_stream_info_result < 0) {
//        //获取失败
//        char *error_info;
//        av_strerror(avformat_find_stream_info_result, error_info, 1024);
//        __android_log_print(ANDROID_LOG_INFO, "main", "异常信息：%s", error_info);
//        return 0;
//    }
//    // Dump valid information onto standard error可忽略
//    av_dump_format(pFormatCtx, 0, cinputFilePath, false);
//
//    //第四步：查找解码器
//    //第一点：获取当前解码器是属于什么类型解码器->找到了视频流
//    //音频解码器、视频解码器、字幕解码器等等...
//    //获取视频解码器流引用->指针
//    int av_stream_index = -1;
//    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
//        //循环遍历每一流
//        //视频流、音频流、字幕流等等...
//        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//            //找到了
//            av_stream_index = i;
//            break;
//        }
//    }
//    if (av_stream_index == -1) {
//        __android_log_print(ANDROID_LOG_INFO, "main", "%s", "没有找到视频流");
//        return 0;
//    }
//
//
//
//    //第二点：根据视频流->查找到视频解码器上下文->视频压缩数据
//    AVCodecContext *avcodec_context = pFormatCtx->streams[av_stream_index]->codec;
//    //第三点：根据解码器上下文->获取解码器ID
//    AVCodec *avcodec = avcodec_find_decoder(avcodec_context->codec_id);
//    if (avcodec == NULL) {
//        __android_log_print(ANDROID_LOG_INFO, "main", "%s", "没有找到视频解码器");
//        return 0;
//    }
//    //第五步：打开解码器
//    int avcodec_open2_result = avcodec_open2(avcodec_context, avcodec, NULL);
//    if (avcodec_open2_result != 0) {
//        char *error_info;
//        av_strerror(avcodec_open2_result, error_info, 1024);
//        __android_log_print(ANDROID_LOG_INFO, "main", "异常信息：%s", error_info);
//        return 0;
//    }
//    //输出视频信息
//    //输出：文件格式
//    __android_log_print(ANDROID_LOG_INFO, "main", "文件格式：%s", pFormatCtx->iformat->name);
//    //输出：解码器名称
//    __android_log_print(ANDROID_LOG_INFO, "main", "解码器名称：%s", avcodec->name);
//    //第六步：循环读取视频帧，进行循环解码->输出YUV420P视频->格式：yuv格式
//    //读取帧数据换成到哪里->缓存到packet里面
//    AVPacket *av_packet = (AVPacket *) av_malloc(sizeof(AVPacket));
//    //输入->环境一帧数据->缓冲区->类似于一张图
//    AVFrame *av_frame_in = av_frame_alloc();
//    //输出->帧数据->视频像素数据格式->yuv420p
//    AVFrame *av_frame_out_yuv420p = av_frame_alloc();
//    //解码的状态类型(0:表示解码完毕，非0:表示正在解码)
//    int av_decode_result, current_frame_index = 0;
//    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
//    //缓冲区
//    //作用：计算音频/视频占用的字节数，开辟对应的内存空间
//    //参数一：缓冲区格式
//    //参数二：缓冲区宽度
//    //参数三：缓冲区高度
//    //参数四：字节对齐(设置通用1)
//    int image_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, avcodec_context->width, avcodec_context->height,1);
//    //开辟缓存空间
//    uint8_t *frame_buffer_out = (uint8_t *)av_malloc(image_size);
//    //对开辟的缓存空间指定填充数据格式
//    //参数一：数据
//    //参数二：行数
//    //参数三：缓存区
//    //参数四：格式
//    //参数五：宽度
//    //参数六：高度
//    //参数七：字节对齐(设置通用1)
//    av_image_fill_arrays(av_frame_out_yuv420p->data, av_frame_out_yuv420p->linesize,frame_buffer_out,
//                         AV_PIX_FMT_YUV420P,avcodec_context->width, avcodec_context->height,1);
//    //准备一个视频像素数据格式上下文
//    //参数一：输入帧数据宽
//    //参数二：输入帧数据高
//    //参数三：输入帧数据格式
//    //参数四：输出帧数据宽
//    //参数五：输出帧数据高
//    //参数六：输出帧数据格式->AV_PIX_FMT_YUV420P
//    //参数七：视频像素数据格式转换算法类型
//    //参数八：字节对齐类型(C/C++里面)->提高读取效率
//    SwsContext *sws_context = sws_getContext(avcodec_context->width,
//                                             avcodec_context->height,
//                                             avcodec_context->pix_fmt,
//                                             avcodec_context->width,
//                                             avcodec_context->height,
//                                             AV_PIX_FMT_YUV420P,
//                                             SWS_BICUBIC, NULL, NULL, NULL);
//    // 加载SDL
//    //第一步：初始化SDL多媒体框架->SDL_Init
//    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
//        LOG_I("SDL_Init failed %s", SDL_GetError());
//        return 0;
//    }
//    LOG_I("SDL_Init Success!");
//    //第二步：初始化SDL窗口
//    //参数一：窗口名称->要求必需是UTF-8编码
//    //参数二：窗口在屏幕上面X坐标
//    //参数三：窗口在屏幕上面Y坐标
//    //参数四：窗口在屏幕上面宽
//    int width = 640;
//    //参数五：窗口在屏幕上面高
//    int height = 352;
//    //参数六：窗口状态(打开的状态:SDL_WINDOW_OPENGL)
//    SDL_Window *sdl_window = SDL_CreateWindow("Dream开车",
//                                              SDL_WINDOWPOS_CENTERED,
//                                              SDL_WINDOWPOS_CENTERED,
//                                              width,
//                                              height,
//                                              SDL_WINDOW_OPENGL);
//    if (sdl_window == NULL) {
//        LOG_I("窗口创建失败");
//        return 0;
//    }
//    //第三步：创建渲染器->渲染窗口(OpenGL ES)
//    //最新一期VIP课程
//    //参数一：渲染目标窗口
//    //参数二：从哪里开始渲染(-1:默认从第一个为止开始)
//    //参数三：渲染类型
//    //SDL_RENDERER_SOFTWARE:软件渲染
//    //...
//    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
//    //第四步：创建纹理
//    //参数一：纹理目标渲染器
//    //参数二：渲染格式
//    //参数三：绘制方式(SDL_TEXTUREACCESS_STREAMING:频繁绘制)
//    //参数四：纹理宽
//    //参数五：纹理高
//    SDL_Texture *sdl_texture = SDL_CreateTexture(sdl_renderer,
//                                                 SDL_PIXELFORMAT_IYUV,
//                                                 SDL_TEXTUREACCESS_STREAMING,
//                                                 width,
//                                                 height);
//    SDL_Rect sdl_rect;
//    sdl_rect.x = 0;
//    sdl_rect.y = 0;
//    sdl_rect.w = width;
//    sdl_rect.h = height;
//    //>=0:说明有数据，继续读取
//    //<0:说明读取完毕，结束
//    while (av_read_frame(pFormatCtx, av_packet) >= 0) {
//        //解码什么类型流(视频流、音频流、字幕流等等...)
//        if (av_packet->stream_index == av_stream_index) {
//            //扩展知识面(有更新)
//            //解码一帧视频流数据
//            //分析：avcodec_decode_video2函数
//            //参数一：解码器上下文
//            //参数二：一帧数据
//            //参数三：got_picture_ptr->是否正在解码(0:表示解码完毕，非0:表示正在解码)
//            //参数四：一帧压缩数据(对压缩数据进行解码操作)
//            //返回值：av_decode_result == 0表示解码一帧数据成功，否则失败
//            //av_decode_result = avcodec_decode_video2(avcodec_context,av_frame_in,&got_picture_ptr,av_packet);
//            //新的API操作
//            //发送一帧数据->接收一帧数据
//            //发送一帧数据
//            avcodec_send_packet(avcodec_context, av_packet);
//            //接收一帧数据->解码一帧
//            av_decode_result = avcodec_receive_frame(avcodec_context, av_frame_in);
//            //解码出来的每一帧数据成功之后，将每一帧数据保存为YUV420格式文件类型(.yuv文件格式)
//            if (av_decode_result == 0) {
//                //sws_scale：作用将视频像素数据格式->yuv420p格式
//                //输出.yuv文件->视频像素数据格式文件->输出到文件API
//                //参数一：视频像素数据格式->上下文
//                //参数二：输入数据
//                //参数三：输入画面每一行的大小
//                //参数四：输入画面每一行的要转码的开始位置
//                //参数五：每一帧数据高
//                //参数六：输出画面数据
//                //参数七：输出画面每一行的大小
//                sws_scale(sws_context,
//                          (const uint8_t *const *) av_frame_in->data,
//                          av_frame_in->linesize,
//                          0,
//                          avcodec_context->height,
//                          av_frame_out_yuv420p->data,
//                          av_frame_out_yuv420p->linesize);
//                //  sart SDL  //
//                //SDL渲染实现
//                //设置纹理数据
//                //参数一：目标纹理对象
//                //参数二：渲染区域(NULL:表示默认屏幕窗口宽高)
//                //参数三：视频像素数据
//                //参数四：帧画面宽
//                SDL_UpdateTexture(sdl_texture, NULL, av_frame_out_yuv420p->data[0],
//                                  av_frame_out_yuv420p->linesize[0]);
//                //先清空
//                SDL_RenderClear(sdl_renderer);
//                //再渲染
//                SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, &sdl_rect);
//                //第七步：显示帧画面
//                SDL_RenderPresent(sdl_renderer);
//                //第八步：延时渲染(没渲染一帧间隔时间)
//                SDL_Delay(20);
//                // end SDL //
//                current_frame_index++;
//                __android_log_print(ANDROID_LOG_INFO, "main", "当前遍历第%d帧", current_frame_index);
//            }
//        }
//    }
//    //第七步：关闭解码组件->释放内存
//    SDL_DestroyTexture(sdl_texture);
//    SDL_DestroyRenderer(sdl_renderer);
//    //第十步：推出SDL程序
//    SDL_Quit();
//    av_packet_free(&av_packet);
//    av_frame_free(&av_frame_in);
//    av_frame_free(&av_frame_out_yuv420p);
//    avcodec_close(avcodec_context);
//    avformat_free_context(pFormatCtx);
//    return 0;
//}
#define NUM_SOUNDS 2
struct sample {
    Uint8 *data;
    Uint32 dpos;
    Uint32 dlen;
} sounds[NUM_SOUNDS];

void mixaudio(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint32 amount;

    for ( i=0; i<NUM_SOUNDS; ++i ) {
        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
        sounds[i].dpos += amount;
    }
}

int sfp_refresh_thread(void *opaque){
    int data_count=0;
    int pcm_buffer_size=4096;
    char *pcm_buffer=(char *)malloc(pcm_buffer_size);
    bool quit=false;
    FILE *pFile=fopen("/storage/emulated/0/Test.pcm", "rb+");
    if(pFile==NULL){
        LOG_I("can't open this file\n");
        return -1;
    }
    while(!quit){
        if (fread(pcm_buffer, 1, pcm_buffer_size, pFile) != pcm_buffer_size){
            // Loop
            fseek(pFile, 0, SEEK_SET);
            fread(pcm_buffer, 1, pcm_buffer_size, pFile);
            data_count=0;
        }
        //        LOG_I("Now Playing %10d Bytes data.\n",data_count);
        data_count+=pcm_buffer_size;
        //Set audio buffer (PCM data)
        audio_chunk = (Uint8 *) pcm_buffer;
        //Audio buffer length
        audio_len =pcm_buffer_size;
        audio_pos = audio_chunk;
        if(audio_len>0){
            SDL_Delay(22);
        } else{
            quit=true;
        }
    }
    free(pcm_buffer);
    SDL_Quit();
    return 0;
}
int main(int argc, char *argv[]) {
 /** 函数调用步骤如下:
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_OpenAudio(): 根据参数（存储于SDL_AudioSpec）打开音频设备。
 * SDL_PauseAudio(): 播放音频数据。
 *
 * [循环播放数据]
 * SDL_Delay(): 延时等待播放完成。
 * */
    //第一步：注册所有组件
    av_register_all();
    //支持网络流输入
    avformat_network_init();
    //第二步：打开视频输入文件
    //参数一：封装格式上下文->AVFormatContext->包含了视频信息(视频格式、大小等等...)
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    const char *cinputFilePath = "/storage/emulated/0/Test.pcm";
    //---------SDL--------------------------------------
    //初始化
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1) {
        LOG_I("SDL_Init failed %s", SDL_GetError());
        return -1;
    }
    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = 32000;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = 2;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024; //播放AAC，M4a，缓冲区的大小
    //wanted_spec.samples = 1152; //播放MP3，WMA时候用
    wanted_spec.callback = fill_audio;
    //打开音频设备
    if (SDL_OpenAudio(&wanted_spec, NULL)<0){
        LOG_I("can't open audio.\n");
        return -1;
    }
    FILE *pFile=fopen("/storage/emulated/0/Test.pcm", "rb+");
    if(pFile==NULL){
        LOG_I("can't open this file\n");
        return -1;
    }
//    SDL_CreateThread(,"play pcm",NULL);
    int pcm_buffer_size=4096;
    char *pcm_buffer=(char *)malloc(pcm_buffer_size);
    int data_count=0;
    //Play
    SDL_PauseAudio(0);
    bool quit=false;
    SDL_Thread *video_tid;
//    video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
    while(!quit){
        if (fread(pcm_buffer, 1, pcm_buffer_size, pFile) != pcm_buffer_size){
            // Loop
            fseek(pFile, 0, SEEK_SET);
            fread(pcm_buffer, 1, pcm_buffer_size, pFile);
            data_count=0;
        }
//        LOG_I("Now Playing %10d Bytes data.\n",data_count);
        data_count+=pcm_buffer_size;
        //Set audio buffer (PCM data)
        audio_chunk = (Uint8 *) pcm_buffer;
        //Audio buffer length
        audio_len =pcm_buffer_size;
        audio_pos = audio_chunk;
        if(audio_len>0){
            SDL_Delay(22);
        } else{
            quit=true;
        }
    }
    free(pcm_buffer);
    SDL_Quit();
    return 0;
}

void PlaySound(char *file)
{
    int index;
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;

    /* 寻找一个空的（或者完成了的）音频口 */
    for ( index=0; index<NUM_SOUNDS; ++index ) {
        if ( sounds[index].dpos == sounds[index].dlen ) {
            break;
        }
    }
    if ( index == NUM_SOUNDS )
        return;

    /* 加载声音文件，并转换成16位、立体声、22kHz格式 */
    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
        fprintf(stderr, "无法加载 %s: %s\n", file, SDL_GetError());
        return;
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
                      AUDIO_S16,   2,             22050);
    cvt.buf = (Uint8 *) malloc(dlen * cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);

    /* 将音频数据放入音频口（立刻开始回放了） */
    if ( sounds[index].data ) {
        free(sounds[index].data);
    }
    SDL_LockAudio();
    sounds[index].data = cvt.buf;
    sounds[index].dlen = cvt.len_cvt;
    sounds[index].dpos = 0;
    SDL_UnlockAudio();
}
