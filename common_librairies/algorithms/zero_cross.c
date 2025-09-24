
//int8_t zeroX_detect(filter_100Hz_t *filter, uint16_t index)
//{
//    static float32_t last_values[8] = {0.0};
//    int8_t ret = 0;
//    uint8_t i = 0;
//
//    for(i = 0; i<1; i++)
//    {
//        if (last_values[i] < 0)
//        {
//            if(filter[i].y[0] > 0)
//            {
//                filter[i].zc_pos = 1;
//                if (filter[i].last_index > index)
//                {
//                    filter[i].samples_between = filter[i].last_index - index;
//                }else
//                {
//                    filter[i].samples_between = index - filter[i].last_index;
//                }
//                GPIO_writePin(69, 1);
//                ret++;
//            }
//            else
//            {
//                filter[i].zc_pos = 0;
//                filter[i].zc_neg = 0;
//            }
//        }
//        else
//        {
//            if(filter[i].y[0] < 0)
//            {
//                filter[i].zc_neg = 1;
//                GPIO_writePin(69, 0);
//            }
//            else
//            {
//                filter[i].zc_pos = 0;
//                filter[i].zc_neg = 0;
//            }
//        }
//        filter[i].last_index = index;
//        last_values[i] = filter[i].y[0];
//    }
//    return ret;
//}
