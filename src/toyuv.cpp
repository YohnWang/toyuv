#include<stdio.h>
#include<vector>
#include<string>
#include<algorithm>
#include<exception>
#include<source_location>
#include<memory>
#include<format>
#include<filesystem>

class yuv_exception:public std::exception
{
    const char *s="yuv default exception";
    std::source_location location;
public:
    yuv_exception(const std::source_location& location = std::source_location::current()):location{location}{}
    yuv_exception(const char *s):s{s}{}
    const char * what() const noexcept
    {
        return s;
    };
};

void fclose_wrapper(FILE *fp)
{
    if(fp)
        fclose(fp);
}

struct rgb_t
{
    double r;
    double g;
    double b;
};

struct yuv_t
{
    double y;
    double u;
    double v;
    
    yuv_t()=default;
    yuv_t(double y,double u,double v):y{y},u{u},v{v}{}
    yuv_t(const yuv_t &)=default;
    yuv_t(yuv_t &&)=default;
    yuv_t(rgb_t p):
        y{std::clamp(0.299 * p.r + 0.587 * p.g + 0.114 * p.b, 0.0, 1.0)},
        u{std::clamp((-0.169) * p.r + (-0.331) * p.g + 0.5 * p.b + 0.5, 0.0, 1.0)},
        v{std::clamp(0.5 * p.r + (-0.418) * p.g + (-0.081) * p.b + 0.5, 0.0, 1.0)}
        {}
    constexpr yuv_t& operator=(const yuv_t &)=default;
    constexpr yuv_t& operator=(yuv_t &&)=default;
};



struct image_rgb_t
{
    std::vector<rgb_t> v;
    int width;
    int height;

    void read_ppm(const char *s)
    {
        FILE *fp=fopen(s,"rb");
        if(!fp)
            throw yuv_exception("image_rgb_t open ppm file failed");
        std::unique_ptr<FILE,decltype(&fclose_wrapper)> ptr(fp,fclose_wrapper);
        char type[8]={};
        fscanf(fp, "%s", type);
        if(std::string(type)=="P3")
        {
            read_ppm_p3(fp);
        }
        else if(std::string(type)=="P6")
        {
            read_ppm_p6(fp);
        }
        else
        {
            throw yuv_exception("ppm format error");
        }

    }

    void dump_rgb24(FILE *fp)
    {
        for(auto &i:v)
        {
            fputc(255*i.r, fp);
            fputc(255*i.g, fp);
            fputc(255*i.b, fp);
        }
    }

    void dump_rgb32(FILE *fp)
    {
        for(auto &i:v)
        {
            fputc(255*i.r, fp);
            fputc(255*i.g, fp);
            fputc(255*i.b, fp);
            fputc(0, fp);
        }
    }
private:
    void read_ppm_p3(FILE *fp)
    {
        int width,height,max;
        fscanf(fp,"%d %d %d", &width, &height, &max);
        this->width=width;
        this->height=height;
        v.clear();
        v.resize(width*height);
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
            {
                int r,g,b;
                fscanf(fp, "%d %d %d", &r, &g, &b);
                v[i*width+j] = rgb_t{1.0*r/max,1.0*g/max,1.0*b/max};
            }
        }
    }
    void read_ppm_p6(FILE *fp)
    {
        int width,height,max;
        fscanf(fp,"%d %d %d", &width, &height, &max);
        this->width=width;
        this->height=height;
        v.clear();
        v.resize(width*height);
        fgetc(fp);
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
            {
                int r=fgetc(fp),g=fgetc(fp),b=fgetc(fp);
                v[i*width+j] = rgb_t{1.0*r/max,1.0*g/max,1.0*b/max};
            }
        }
    }
};

struct image_yuv_t
{
    std::vector<yuv_t> v;
    int width;
    int height;

    image_yuv_t()=default;
    image_yuv_t(const image_yuv_t&)=default;
    image_yuv_t(image_yuv_t&&)=default;
    image_yuv_t(const image_rgb_t &rgb)
    {
        width=rgb.width;
        height=rgb.height;
        v.resize(rgb.v.size());
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                v[i*width+j] = rgb.v[i*width+j];
        }
    }
    image_yuv_t(yuv_t x, int width, int height):width{width},height{height}
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                v.push_back(x);
        }
    }
    image_yuv_t& operator=(const image_yuv_t&)=default;
    image_yuv_t& operator=(image_yuv_t&&)=default;

    void for_each(const auto &f)
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                f(i,j);
        }
    }

    void dump_yuv444p(FILE *fp)
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                fputc(255*v[i*width+j].y, fp);
        }
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                fputc(255*v[i*width+j].u, fp);
        }
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                fputc(255*v[i*width+j].v, fp);
        }
    }

    void dump_yuv444(FILE *fp)
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
            {
                fputc(255*v[i*width+j].y, fp);
                fputc(255*v[i*width+j].u, fp);
                fputc(255*v[i*width+j].v, fp);
            }  
        }
    }

    void dump_yuv420sp(FILE *fp)
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                fputc(255*v[i*width+j].y, fp);
        }
        for(int i=0;i<height;i+=2)
        {
            for(int j=0;j<width;j+=2)
            {
                double au,av;
                if(i+1<height && j+1<width)
                {
                    au = (v[i*width+j].u+v[i*width+j+1].u+v[(i+1)*width+j].u+v[(i+1)*width+j+1].u)/4;
                    av = (v[i*width+j].v+v[i*width+j+1].v+v[(i+1)*width+j].v+v[(i+1)*width+j+1].v)/4;
                }
                else
                {
                    au = v[i*width+j].u;
                    av = v[i*width+j].v;
                }
                fputc(255*au, fp);
                fputc(255*av, fp);
            }
        }
    }

    void dump_nv12(FILE *fp)
    {
        dump_yuv420sp(fp);
    }

    void dump_yuv422sp(FILE *fp)
    {
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
                fputc(255*v[i*width+j].y, fp);
        }
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j+=2)
            {
                double au,av;
                if(j+1<width)
                {
                    au=(v[i*width+j].u+v[i*width+j+1].u)/2;
                    av=(v[i*width+j].v+v[i*width+j+1].v)/2;
                }
                else
                {
                    au=v[i*width+j].u;
                    av=v[i*width+j].v;
                }

                fputc(255*au, fp);
                fputc(255*av, fp);
            }
        }
    }

    void dump_nv16(FILE *fp)
    {
        dump_yuv422sp(fp);
    }
};


int main(int argc, const char *argv[])
{
    if(argc<2)
    {
        throw yuv_exception("toyuv need a name of ppm file");
    }

    std::string format="nv12";
    std::string output;
    std::string input; 
    bool quiet=false;

    for(int i=1;i<argc;i++)
    {
        if(std::string_view(argv[i])=="-f" || std::string_view(argv[i])=="--format")
        {
            format=argv[i+1];
            i++;
        }
        else if(std::string_view(argv[i])=="-o")
        {
            output=argv[i+1];
            i++;
        }
        else if(std::string_view(argv[i])=="-q"||std::string_view(argv[i])=="--quiet")
        {
            quiet=true;
        }
        else
        {
            input=argv[i];
        }
    }

    image_rgb_t src;
    src.read_ppm(input.c_str());

    if(output.empty())
    {
        namespace fs = std::filesystem;
        fs::path input_path=input;
        fs::path input_basename=input_path.filename();
        output=std::format("{}.{}x{}.{}",input_basename.string(),src.width,src.height,format);
    }

    image_yuv_t dst=src;
    FILE *fp=fopen(output.c_str(),"wb");
    if(!fp)
    {
        throw yuv_exception("create file failed");
    }

    if(format=="nv12"||format=="yuv420sp")
        dst.dump_nv12(fp);
    else if(format=="nv16"||format=="yuv422sp")
        dst.dump_nv16(fp);
    fclose(fp);
    
    if(!quiet)
        printf("%d %d\n", src.width, src.height);
}