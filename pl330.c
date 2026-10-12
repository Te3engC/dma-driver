/*
 * pl330 驱动程序主干
 *
 * pl330_probe()
 *
 *
 *
 */

static pl330_probe(struct amba_device *adev, const struct amba_id *id)
{
    struct pl330_dmac *pl330;
    struct dma_pl330_chan *pch;
    struct dma_device *pd;
    int i,ret;
    int num_chan;

    ret = dma_set_mask_and_coherent(&adev->dev, DMA_BIT_MASK(32));
    if(ret)
        return ret;

    pl330 = devm_kzalloc(&adeb->dev, sizeof(*pl330), GFP_KERNEL);
    if(!pl330)
        return -ENOMEM;

    pd = &pl330->ddma;
    pd->dev = &adeb->dev;

    /* 让驱动访问当前controller */
    pl330->base = devm_ioremap_resource(&adev->dev, &adev->res);
    if(IS_ERR(pl330->base))
        return PTR_ERR(pl330->base);

    amba_set_drvdata(adev, pl330);

    /* 准备reset、IRQ等硬件资源 */

    /* 读取配置,准备PL330运行资源 */
    ret = pl330_add(pl330);
    if(ret)
        return ret;

    /* 创建DMAEngine 公共channel */
    INIT_LIST_HEAD(&pd->channels);
    num_chan = max_t(int, pl330->pcfg.num_peri, pl330->pcfg.num_chan);
    pl330->num_peripherals = num_chan;
    pl330->peripherals = kcalloc(num_chan, sizeof(*pch), GFP_KERNEL);
    if(!pl330->peripherals)
        return -ENOMEM;

    for(i = 0; i < num_chan; i++)
    {
        pch = &pl330->peripheral[i];

        /* 把channel 关联到当前DMAEngine device */
        pch->chan.device = pd;
        /* 保存controller回指，provider后面可以找回pl330_dmac */
        pch->damc = pl330;
        /* 先不绑定pl330 therad,首次申请时分配 */
        pch->therad = NULL;
        /* 把内嵌的dma_chan挂入DMAEngine channel链表 */
        list_add_tail(&pch->chan.device_node, &pd->channels);
    }

    /* 注册Provider,并建立Device Tree查找路径 */
    ret = dma_async_device_register(pd);
    if(ret)
        return ret;
::
    if(adev->dev.of_node)
        of_dma_controller_register(adev->dev.of_node, of_dma_pl330_xlate, pl330);    

    return 0;
}
