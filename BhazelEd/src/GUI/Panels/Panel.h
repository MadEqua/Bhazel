#pragma once


namespace BZ {

class Panel {
  public:
    virtual ~Panel() = default;

    void render();

  protected:
    virtual void internalRender() = 0;

    bool open = true;
};

}
