#include <iostream>
#include <bits/this_thread_sleep.h>
#include<matplot/matplot.h>

int main(int argc, char** argv)
{

    using namespace matplot;
    auto f = figure(false);
    auto aux = f->current_axes();
    aux->x_axis().visible(false);
    auto y = std::vector<double>({1, 2, 4, 5, 6, 7});

    auto p = aux->plot(y);

    f->draw();
    std::this_thread::sleep_for(std::chrono::seconds(3L));
    y.push_back(10);
    aux->plot(y);
    f->show();

    return 0;
}
