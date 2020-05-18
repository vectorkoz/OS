#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <ctime>
#include <functional>
#include <iomanip>
#include <ntdef.h>


struct Matrix
{
    Matrix(int rows, int columns) : rows(rows), columns(columns) {
        data.assign(rows,std::vector<int>(columns, 0));
    }

    Matrix() {}

    int rows, columns;
    std::vector<std::vector<int>> data;
};

class Wrapper
{
    Matrix one,two,ans;
    int thread_amount, delta_one_row, delta_one_col, delta_two_col;
    int one_row_last_sec_start, one_col_last_sec_start, two_col_last_sec_start;
    std::vector<std::thread> threads;
public:
    Wrapper (int a, int b, int c, int k)
    {
        one=Matrix(a,b);
        two=Matrix(b,c);
        ans=Matrix(a,c);
        thread_amount=k;
    }
    void read(std::fstream &fstream)
    {
        int tmp;
        for (int i=0; i<one.rows; i++)
        {
            for (int j=0; j<one.columns;j++)
            {
                fstream>>tmp;
                one.data[i][j]=tmp;
            }
        }
        for (int i=0; i<two.rows; i++)
        {
            for (int j=0; j<two.columns;j++)
            {
                fstream>>tmp;
                two.data[i][j]=tmp;
            }
        }
    }
    void output()
    {
        for (int i=0; i<ans.rows; i++)
        {
            for (int j=0; j<ans.columns;j++)
            {
                std::cout<<std::setw(5)<<ans.data[i][j];
            }
            std::cout<<std::endl;
        }
    }
    void simple()
    {
        double duration=time([&]() {simple_multiplication();});
        std::cout<<"Simple multiplication result:"<<std::endl;
        output();
        std::cout<<"Simple multiplication time: "<<duration<<std::endl;
        clear();
    }
    void row_col()
    {
        double duration=time([&]() {row_col_threading();});
        std::cout<<"Row-col result:"<<std::endl;
        output();
        std::cout<<"Row-col time: "<<duration<<std::endl;
        clear();
    }
    void col_row()
    {
        double duration=time([&]() {col_row_threading();});
        std::cout<<"Col-row result:"<<std::endl;
        output();
        std::cout<<"Col-row time: "<<duration<<std::endl;
        clear();
    }
    void col__and_row()
    {
        double duration=time([&]() {col_and_row_threading();});
        std::cout<<"Col and row result:"<<std::endl;
        output();
        std::cout<<"Col and row time: "<<duration<<std::endl;
        clear();
    }


private:
    double time(const std::function<void()>& function) {
        std::clock_t start=std::clock();
        function();
        return (start) / (double) CLOCKS_PER_SEC;
    }
    void clear()
    {
        ans.data.clear();
        ans.data.assign(ans.rows,std::vector<int>(ans.columns, 0));
        threads.clear();
    }
    void simple_multiplication()
    {
        for (int i=0; i<one.rows; i++)
        {
            for (int j=0; j<two.columns; j++)
            {
                for (int k=0; k<one.columns; k++)
                {
                    ans.data[i][j]+=one.data[i][k]*two.data[k][j];
                }
            }
        }
    }
    void row_col_multiplication(int one_row_start, int one_row_end, int two_col_start, int two_col_end)
    {
        if (one_row_start==one_row_last_sec_start)
            one_row_end=one.rows;
        if (two_col_start==two_col_last_sec_start)
            two_col_end=two.columns;

        for (int i=one_row_start; i<one_row_end; i++)
        {
            for (int j=two_col_start; j<two_col_end; j++)
            {
                for (int k=0; k<one.columns; k++)
                {
                    ans.data[i][j]+=one.data[i][k]*two.data[k][j];
                }
            }
        }
    }
    void row_col_threading()
    {
        int k=thread_amount;
        if (k>std::min(one.rows,two.columns))
        {
            k=std::min(one.rows,two.columns);
        }
        delta_one_row=one.rows/k;
        one_row_last_sec_start=delta_one_row*(k-1);
        delta_two_col=two.columns/k;
        two_col_last_sec_start=delta_two_col*(k-1);

        auto function = [&](int i, int j) {
            row_col_multiplication(delta_one_row*i,delta_one_row*(i+1),delta_two_col*j,delta_two_col*(j+1));
        };

        for (int i = 0; i < k; ++i) {
            threads.clear();
            for (int j = 0; j < k; ++j) {
                threads.emplace_back(function, i, j);
            }
            for (auto &t : threads) {
                t.join();
            }
        }
    }
    void col_row_multiplication(int start, int end)
    {
        if (start==one_col_last_sec_start)
            end=one.columns;

        for (int i=0; i<one.rows; i++)
        {
            for (int j=0; j<two.columns; j++)
            {
                for (int k=start; k<end; k++)
                {
                    ans.data[i][j]+=one.data[i][k]*two.data[k][j];
                }
            }
        }
    }
    void col_row_threading()
    {
        int k=thread_amount;
        if (k>one.columns)
        {
            k=one.columns;
            delta_one_col=1;
            one_col_last_sec_start=k-1;
        }
        else
        {
            delta_one_col=one.columns/k;
            one_col_last_sec_start=delta_one_col*(k-1);
        }
        auto function = [&](int i) {
            col_row_multiplication(delta_one_col*i,delta_one_col*(i+1));
        };

        threads.clear();
        for (int j = 0; j < k; ++j) {
            threads.emplace_back(function, j);
        }
        for (auto &t : threads) {
            t.join();
        }
    }
    void col_and_row_multiplication(int one_row_start, int one_row_end, int two_col_start, int two_col_end, int one_col_start, int one_col_end)
    {
        if (one_row_start==one_row_last_sec_start)
            one_row_end=one.rows;
        if (two_col_start==two_col_last_sec_start)
            two_col_end=two.columns;
        if (one_col_start==one_col_last_sec_start)
            one_col_end=one.columns;

        for (int i=one_row_start; i<one_row_end; i++)
        {
            for (int j=two_col_start; j<two_col_end; j++)
            {
                for (int k=one_col_start; k<one_col_end; k++)
                {
                    ans.data[i][j]+=one.data[i][k]*two.data[k][j];
                }
            }
        }
    }
    void col_and_row_threading()
    {
        int k=thread_amount, min=std::min(std::min(one.rows, one.columns),two.columns);
        if (k>min)
        {
            k=min;
        }
        delta_one_row=one.rows/k;
        one_row_last_sec_start=delta_one_row*(k-1);
        delta_two_col=two.columns/k;
        two_col_last_sec_start=delta_two_col*(k-1);
        delta_one_col=one.columns/k;
        one_col_last_sec_start=delta_one_col*(k-1);

        auto function = [&](int i, int j, int q) {
            col_and_row_multiplication(delta_one_row*i,delta_one_row*(i+1),delta_two_col*j,delta_two_col*(j+1),delta_one_col*q,delta_one_col*(q+1));
        };
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) {
                threads.clear();
                for (int q = 0; q < k; ++q) {
                    threads.emplace_back(function, i, j, q);
                }
                for (auto &t : threads) {
                    t.join();
                }
            }
        }
    }
};

int main() {
    std::fstream f("input.txt",std::ios::in);
    int a,b,c,k;
    f>>a>>b>>c>>k;
    Wrapper wrapper(a,b,c,k);
    wrapper.read(f);
    wrapper.simple();
    wrapper.row_col();
    wrapper.col_row();
    wrapper.col__and_row();
    return 0;
}