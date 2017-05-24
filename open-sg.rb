require 'formula'

class OpenSg <Formula
  #head 'cvs://:pserver:anonymous@opensg.cvs.sourceforge.net:/cvsroot/opensg:OpenSG'
  #head 'https://github.com/bilke/OpenSG-1.8.git'
  head 'https://github.com/vossg/OpenSG1x.git'
  homepage 'http://www.opensg.org/wiki'

  depends_on 'libtiff'
  depends_on 'jpeg'
  depends_on 'flex'
  depends_on 'bison'
  depends_on "freetype"
  depends_on "cmake" => :build


  def install
    args = std_cmake_args + %W[
      -DOSGBUILD_OSGWindowGLUT=OFF
      -DOSGBUILD_TESTS=OFF
      -DOSGBUILD_OSGWindowCarbon=OFF
    ]

    mkdir "build" do
      args << ".."
      system "cmake", *args
      system "cmake", "."
      system "make"
      system "make", "install"
    end
  end
end
