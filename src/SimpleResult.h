/*
obs-websocket
Copyright (C) 2016-2020	Stéphane Lepin <stephane.lepin@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#pragma once

#include <stdexcept>

template <typename T>
class SimpleResult {
public:
    static SimpleResult Ok(T result) {
        return SimpleResult(result, true);
    }
    static SimpleResult Error(T result) {
        return SimpleResult(result, false);
    }

    explicit SimpleResult(T result, bool ok) :
        _result(result),
        _ok(ok)
    {};

    bool isOk() {
        return _ok;
    };
    
    T get() {
        return _result;
    };

private:
    bool _ok;
    T _result;
};

// namespace Result {
//     template <typename T, typename E>
//     class Result {
//     public:
//         virtual bool isOk() = 0;
//         virtual T result() = 0;
//         virtual E error() = 0;
//     };

//     template <typename T, typename E>
//     class Ok : Result<T,E> {
//     public:
//         explicit Ok(T result) : _result(result) {}

//         bool isOk() {
//             return true;
//         }
//         T result() {
//             return _result;
//         }
//         E error() {
//             throw std::runtime_error("Error not available");
//         }
//     private:
//         T _result;
//     };

//     template <typename T, typename E>
//     class Error : Result<T,E> {
//     public:
//         explicit Error(E error) : _error(error) {}

//         bool isOk() {
//             return false;
//         }
//         T result() {
//             throw std::runtime_error("Result not available");
//         }
//         E error() {
//             return _error;
//         }
//     private:
//         E _error;
//     };
// };